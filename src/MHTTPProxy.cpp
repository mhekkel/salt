/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023 Maarten L. Hekkelman
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "MHTTPProxy.hpp"
#include "MError.hpp"
#include "MPreferences.hpp"
#include "MSaltApp.hpp"

#include <zeep/crypto.hpp>
#include <zeep/http/error-handler.hpp>
#include <zeep/http/html-controller.hpp>
#include <zeep/http/login-controller.hpp>
#include <zeep/http/message-parser.hpp>
#include <zeep/http/security.hpp>
#include <zeep/http/server.hpp>
#include <zeep/http/uri.hpp>

#include <pinch.hpp>

#include <fstream>

// --------------------------------------------------------------------

using tcp = asio_ns::ip::tcp;
namespace zh = zeep::http;
namespace fs = std::filesystem;
using json = zeep::json::element;

// --------------------------------------------------------------------

using asio_ns::ip::tcp;

class proxy_controller;

// --------------------------------------------------------------------

class MHTTPProxyImpl
{
  public:
	MHTTPProxyImpl(std::shared_ptr<pinch::basic_connection> inConnection, uint16_t inPort,
		bool require_authentication, const std::string &user, const std::string &password, log_level log);

	~MHTTPProxyImpl();

	void log_request(const std::string &client,
		const zh::request &request, const std::string &request_line,
		const zh::reply &reply);

	void set_log_level(log_level log_flags);
	void log_error(const std::exception &e);
	void log_error(const std::error_code &ec);

  private:
	asio_ns::io_context m_io_context;
	zeep::http::simple_user_service m_user_service;
	std::shared_ptr<pinch::basic_connection> m_connection;
	std::unique_ptr<zeep::http::basic_server> m_server;
	log_level m_log_level = log_level::none;
	std::unique_ptr<std::ostream> m_log;
};

// --------------------------------------------------------------------

class proxy_controller : public zeep::http::html_controller
{
  public:
	proxy_controller(std::shared_ptr<pinch::basic_connection> ssh_connection, MHTTPProxyImpl &proxy)
		: m_connection(ssh_connection)
		, m_proxy(proxy)
	{
		mount_get("status", &proxy_controller::handle_status);
		mount_get("css/", &proxy_controller::handle_file);
	}

	~proxy_controller()
	{
	}

	bool dispatch_request(tcp::socket &socket, zh::request &req, zh::reply &reply) override
	{
		m_socket = &socket;
		return zh::html_controller::dispatch_request(socket, req, reply);
	}

	void handle_status(const zeep::http::request &req, const zeep::http::scope &scope, zeep::http::reply &rep)
	{
		zh::scope sub(scope);

		json stats{
			{ { "name", "Channels created" },
				{ "value", m_channel_count } },
			{ { "name", "Channels open" },
				{ "value", static_cast<uint32_t>(m_open_channel_count) } },
			{ { "name", "Requests processed" },
				{ "value", m_request_count } },
		};
		sub.put("stats", stats);

		get_template_processor().create_reply_from_template("templates/status.html", sub, rep);
	}

	bool handle_request(zh::request &req, zh::reply &reply) override
	{
		bool result = true;

		++m_request_count;

		if (zh::uri(req.get_uri()).get_host() == "proxy.hekkelman.net")
			result = zh::html_controller::handle_request(req, reply);
		else if (req.get_method() == "CONNECT")
			asio_ns::co_spawn(
				m_connection->get_executor(), [this, req]()
				{ return handle_connect(req, std::move(*m_socket)); },
				asio_ns::detached);
		else
			asio_ns::co_spawn(
				m_connection->get_executor(), [this, req]()
				{ return handle_proxy_requests(req, std::move(*m_socket)); },
				asio_ns::detached);

		return result;
	}

	struct open_channel_counter
	{
		open_channel_counter(std::atomic<uint32_t> &cnt)
			: m_cnt(cnt)
		{
			++m_cnt;
		}

		~open_channel_counter()
		{
			--m_cnt;
		}

		std::atomic<uint32_t> &m_cnt;
	};

	struct connect_copy : public std::enable_shared_from_this<connect_copy>
	{
		tcp::socket socket;
		std::shared_ptr<pinch::forwarding_channel> channel;
		open_channel_counter cnt;

		connect_copy(tcp::socket &&socket, std::shared_ptr<pinch::forwarding_channel> channel, std::atomic<uint32_t> &cnt)
			: socket(std::forward<tcp::socket>(socket))
			, channel(channel)
			, cnt(cnt)
		{
		}

		template <typename SocketIn, typename SocketOut>
		asio_ns::awaitable<void> copy(SocketIn &in, SocketOut &out)
		{
			char data[1024];
			std::error_code ec;

			try
			{
				for (;;)
				{
					auto length = co_await asio_ns::async_read(in, asio_ns::buffer(data), asio_ns::transfer_at_least(1), asio_ns::use_awaitable);
					if (length == 0)
						break;
					co_await asio_ns::async_write(out, asio_ns::buffer(data, length), asio_ns::use_awaitable);
				}
			}
			catch (...)
			{
			}
		}

		void start()
		{
			auto self = shared_from_this();
			asio_ns::co_spawn(
				socket.get_executor(), [self]()
				{ return self->copy(self->socket, *self->channel); },
				asio_ns::detached);
			asio_ns::co_spawn(
				socket.get_executor(), [self]()
				{ return self->copy(*self->channel, self->socket); },
				asio_ns::detached);
		}
	};

	asio_ns::awaitable<void> handle_connect(zh::request req, tcp::socket socket)
	{
		std::string host = req.get_uri().get_host();

		uint16_t port = req.get_uri().get_port();
		if (port == 0)
			port = 80;

		std::string client;
		try // asking for the remote endpoint address failed sometimes
		    // causing aborting exceptions, so I moved it here.
		{
			client = socket.remote_endpoint().address().to_string();
		}
		catch (...)
		{
			client = "unknown";
		}

		++m_request_count;

		auto channel = std::make_shared<pinch::forwarding_channel>(m_connection, host, port);
		++m_channel_count;

		co_await channel->async_open(asio_ns::use_awaitable);

		zh::reply reply(zeep::http::ok);
		asio_ns::streambuf buffer;
		std::ostream out(&buffer);
		out << reply;

		m_proxy.log_request(client, req, req.get_request_line(), reply);
		co_await asio_ns::async_write(socket, buffer, asio_ns::use_awaitable);

		std::make_shared<connect_copy>(std::move(socket), channel, m_open_channel_count)->start();
	}

	asio_ns::awaitable<void> send_reply(tcp::socket &socket, const zh::reply &reply)
	{
		asio_ns::streambuf buffer;
		std::ostream out(&buffer);
		out << reply;

		co_await asio_ns::async_write(socket, buffer, asio_ns::use_awaitable);
	}

	asio_ns::awaitable<void> handle_proxy_requests(zh::request req, tcp::socket socket)
	{
		std::error_code ec;
		std::shared_ptr<pinch::forwarding_channel> channel;
		open_channel_counter cnt(m_open_channel_count);

		while (not ec)
		{
			auto uri = req.get_uri();

			// std::regex re("^(?:http://)?(?:([-$_.+!*'(),[:alnum:];?&=]+)(?::([-$_.+!*'(),[:alnum:];?&=]+))?@)?([-[:alnum:].]+)(?::(\\d+))?(/.*)?");
			// std::smatch mr;

			// std::string uri = req.get_uri();
			// if (not std::regex_match(uri, mr, re))
			// {
			// 	send_reply(socket, zh::reply{zh::bad_request}, yield);
			// 	return;
			// }

			++m_request_count;

			std::string host = uri.get_host();
			uint16_t port = uri.get_port();

			if (port == 0)
				port = 80;

			// m_proxy.validate(m_request);

			if (not(channel and channel->forwards_to(host, port)))
				channel.reset(new pinch::forwarding_channel(m_connection, host, port));

			if (not channel->is_open())
			{
				co_await channel->async_open(asio_ns::use_awaitable);
				++m_channel_count;
			}

			asio_ns::streambuf buffer;
			std::ostream out(&buffer);
			out << req;

			co_await asio_ns::async_write(*channel, buffer, asio_ns::use_awaitable);

			zeep::http::reply_parser rep_parser;
			zh::parse_result r;

			do
			{
				auto buf = buffer.prepare(1024);
				std::size_t n = co_await channel->async_read_some(buf, asio_ns::use_awaitable);
				buffer.commit(n);
				r = rep_parser.parse(buffer);
			} while (r == zh::indeterminate);

			auto reply = rep_parser.get_reply();

			out.clear();
			out << reply;
			co_await asio_ns::async_write(socket, buffer, asio_ns::use_awaitable);

			// restart with a next request unless it is HTTP/1.0
			const auto &[major, minor] = req.get_version();
			if (minor == 0)
				break;

			zeep::http::request_parser req_parser;

			do
			{
				auto buf = buffer.prepare(1024);
				std::size_t n = co_await asio_ns::async_read(socket, buf, asio_ns::transfer_at_least(1), asio_ns::use_awaitable);
				buffer.commit(n);
				r = req_parser.parse(buffer);
			} while (r == zh::indeterminate);

			req = req_parser.get_request();
		}
	}

  private:
	std::shared_ptr<pinch::basic_connection> m_connection;
	tcp::socket *m_socket = nullptr;
	MHTTPProxyImpl &m_proxy;
	std::atomic<uint32_t> m_open_channel_count = 0;
	uint32_t m_channel_count = 0, m_request_count = 0;
};

// --------------------------------------------------------------------

class http_proxy_error_handler : public zeep::http::error_handler
{
  public:
	http_proxy_error_handler()
		: error_handler("not-found.html")
	{
	}

	virtual bool create_error_reply(const zeep::http::request &req, std::exception_ptr eptr, zeep::http::reply &reply);
};

bool http_proxy_error_handler::create_error_reply(const zeep::http::request &req, std::exception_ptr eptr, zeep::http::reply &reply)
{
	bool result = false;

	try
	{
		if (eptr)
			std::rethrow_exception(eptr);
	}
	catch (const std::error_code &ec)
	{
		result = zeep::http::error_handler::create_error_reply(req, zeep::http::internal_server_error, ec.message(), reply);
	}
	catch (const zeep::http::status_type &s)
	{
		if (s == zeep::http::not_found)
			result = zeep::http::error_handler::create_error_reply(req, s, zeep::http::get_status_description(s), reply);
	}

	return result;
}

// --------------------------------------------------------------------

class MHTTPServer : public zeep::http::basic_server
{
  public:
	MHTTPServer(MHTTPProxyImpl &proxy, asio_ns::io_context &io_context, zeep::http::security_context *sctxt)
		: zeep::http::basic_server(sctxt)
		, m_io_context(io_context)
		, m_proxy(proxy)
	{
	}

	asio_ns::io_context &get_io_context() override
	{
		return m_io_context;
	}

	void log_request(const std::string &client, const zh::request &req, const zh::reply &rep,
		const std::chrono::system_clock::time_point &pt, const std::string &referer,
		const std::string &userAgent, const std::string &entry) noexcept override
	{
		m_proxy.log_request(client, req, req.get_request_line(), rep);
	}

  private:
	asio_ns::io_context &m_io_context;
	MHTTPProxyImpl &m_proxy;
};

// --------------------------------------------------------------------

MHTTPProxyImpl::MHTTPProxyImpl(std::shared_ptr<pinch::basic_connection> inConnection, uint16_t inPort,
	bool require_authentication, const std::string &user, const std::string &password, log_level log)
	: m_user_service({ { user, password, { "PROXY_USER" } } })
	, m_connection(inConnection)
	, m_log_level(log_level::none)
{
#if NDEBUG
	set_log_level(log);
#else
	set_log_level(log_level::debug);
#endif

	// std::string secret = zeep::encode_base64(zeep::random_hash());
	// auto sc = new zeep::http::security_context(secret, m_user_service, not require_authentication);

	// sc->add_rule("/status", "PROXY_USER");
	// sc->add_rule("/", {});

	// m_server.reset(new MHTTPServer(gApp->get_io_context(), sc));
	m_server.reset(new MHTTPServer(*this, MSaltApp::Instance().get_io_context(), nullptr));

	m_server->set_allowed_methods({ "GET", "POST", "PUT", "OPTIONS", "HEAD", "DELETE", "CONNECT" });

	m_server->add_error_handler(new http_proxy_error_handler());

	m_server->set_template_processor(new zeep::http::rsrc_based_html_template_processor(""));

	// m_server->add_controller(new zeep::http::login_controller());
	m_server->add_controller(new proxy_controller(m_connection, *this));

	m_server->bind("localhost", inPort);
}

MHTTPProxyImpl::~MHTTPProxyImpl()
{
	if (m_server)
		m_server->stop();
}

void MHTTPProxyImpl::set_log_level(log_level level)
{
	m_log_level = level;

	if (level > log_level::none)
		m_log.reset(new std::ofstream(gPrefsDir / "proxy.log", std::ios::app));
	else
		m_log.reset(nullptr);
}

void MHTTPProxyImpl::log_request(const std::string &client, const zh::request &request, const std::string &request_line, const zh::reply &reply)
{
	try
	{
		if (m_log_level >= log_level::request)
		{
			std::string referer = request.get_header("Referer");
			if (referer.empty())
				referer = "-";

			std::string userAgent = request.get_header("User-Agent");
			if (userAgent.empty())
				userAgent = "-";

			const std::time_t now_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

			*m_log << client << ' '
				   << "-" << ' '
				   << "-" << ' '
				   << std::put_time(std::localtime(&now_t), "[%d/%b/%Y:%H:%M:%S %z]") << ' '
				   << '"' << request_line << "\" "
				   << std::to_string(reply.get_status()) << ' '
				   << reply.size() << ' '
				   << '"' << referer << '"' << ' '
				   << '"' << userAgent << '"'
				   << '\n'
				   << std::flush;
		}
	}
	catch (...)
	{
	}
}

void MHTTPProxyImpl::log_error(const std::exception &e)
{
#if DEBUG
	std::cerr << "ERROR (ex): " << e.what() << '\n';
#endif

	if (m_log)
		*m_log << "ERROR: " << e.what() << '\n';
}

void MHTTPProxyImpl::log_error(const std::error_code &ec)
{
#if DEBUG
	std::cerr << "ERROR (ec): " << ec.message() << '\n';
#endif

	if (m_log and
		ec != pinch::error::make_error_code(pinch::error::channel_closed) and
		ec != pinch::error::make_error_code(pinch::error::connection_lost) and
		ec != asio_ns::error::make_error_code(asio_ns::error::eof))
	{
		*m_log << "ERROR: " << ec.message() << '\n';
	}
}

// --------------------------------------------------------------------

MHTTPProxy::MHTTPProxy()
	: m_impl(nullptr)
{
}

MHTTPProxy::~MHTTPProxy()
{
	delete m_impl;
}

MHTTPProxy &MHTTPProxy::instance()
{
	static std::unique_ptr<MHTTPProxy> s_instance(new MHTTPProxy);
	return *s_instance;
}

void MHTTPProxy::Init(std::shared_ptr<pinch::basic_connection> inConnection,
	uint16_t inPort, bool require_authentication, log_level log)
{
	if (m_impl)
		delete m_impl;

	auto user = MPrefs::GetString("http-proxy-user", "");
	auto password = MPrefs::GetString("http-proxy-password", "");

	m_impl = new MHTTPProxyImpl(inConnection, inPort, require_authentication, user, password, log);
}
