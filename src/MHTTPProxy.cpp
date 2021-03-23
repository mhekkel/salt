//           Copyright Maarten L. Hekkelman 2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MSalt.hpp"

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <pinch/port_forwarding.hpp>

#include <zeep/crypto.hpp>
#include <zeep/http/error-handler.hpp>
#include <zeep/http/html-controller.hpp>
#include <zeep/http/login-controller.hpp>
#include <zeep/http/message-parser.hpp>
#include <zeep/http/security.hpp>
#include <zeep/http/server.hpp>

#include "MApplication.hpp"
#include "MError.hpp"
#include "MHTTPProxy.hpp"
#include "MPreferences.hpp"

// --------------------------------------------------------------------

using tcp = boost::asio::ip::tcp;
namespace zh = zeep::http;
namespace fs = std::filesystem;
using json = zeep::json::element;

// --------------------------------------------------------------------

using namespace boost::posix_time;

using boost::asio::ip::tcp;

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
	void log_error(const boost::system::error_code &ec);

  private:
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
			{
				{ "name", "Channels created" },
				{ "value", m_channel_count }
			},
			{
				{ "name", "Channels open" },
				{ "value", static_cast<uint32_t>(m_open_channel_count) }
			},
			{
				{ "name", "Requests processed" },
				{ "value", m_request_count }
			},
		};
		sub.put("stats", stats);

		get_template_processor().create_reply_from_template("templates/status.html", sub, rep);
	}

	bool handle_request(zh::request &req, zh::reply &reply) override
	{
		bool result = true;

		++m_request_count;

		if (req.get_host() == "proxy.hekkelman.net")
			result = zh::html_controller::handle_request(req, reply);
		else if (req.get_method() == "CONNECT")
			boost::asio::spawn(m_connection->get_executor(), [this, req](boost::asio::yield_context yield) { handle_connect(req, std::move(*m_socket), yield); });
		else
			boost::asio::spawn(m_connection->get_executor(), [this, req](boost::asio::yield_context yield) { handle_proxy_requests(req, std::move(*m_socket), yield); });

		return result;
	}

	struct open_channel_counter
	{
		open_channel_counter(std::atomic<uint32_t>& cnt)
			: m_cnt(cnt)
		{
			++m_cnt;
		}

		~open_channel_counter()
		{
			--m_cnt;
		}

		std::atomic<uint32_t>& m_cnt;
	};

	struct connect_copy : public std::enable_shared_from_this<connect_copy>
	{
		tcp::socket socket;
		std::shared_ptr<pinch::forwarding_channel> channel;
		open_channel_counter cnt;

		connect_copy(tcp::socket &&socket, std::shared_ptr<pinch::forwarding_channel> channel, std::atomic<uint32_t>& cnt)
			: socket(std::forward<tcp::socket>(socket))
			, channel(channel)
			, cnt(cnt)
		{
		}

		template<typename SocketIn, typename SocketOut>
		void copy(SocketIn& in, SocketOut& out, std::shared_ptr<connect_copy> self, boost::asio::yield_context yield)
		{
			char data[1024];
			boost::system::error_code ec;

			while (not ec)
			{
				auto length = boost::asio::async_read(in, boost::asio::buffer(data), boost::asio::transfer_at_least(1), yield[ec]);
				if (length == 0 or ec)
					break;
				boost::asio::async_write(out, boost::asio::buffer(data, length), yield[ec]);
			}
		}

		void start()
		{
			auto self = shared_from_this();
			boost::asio::spawn(socket.get_executor(), [self](boost::asio::yield_context yield) { self->copy(self->socket, *self->channel, self, yield); });
			boost::asio::spawn(socket.get_executor(), [self](boost::asio::yield_context yield) { self->copy(*self->channel, self->socket, self, yield); });
		}
	};

	void handle_connect(zh::request req, tcp::socket socket, boost::asio::yield_context yield)
	{
		std::string host = req.get_uri();
		uint16_t port = 443;

		auto cp = host.find(':');
		if (cp != std::string::npos)
		{
			port = std::stoi(host.substr(cp + 1));
			host.erase(cp, std::string::npos);
		}

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

		boost::system::error_code ec;

		auto channel = std::make_shared<pinch::forwarding_channel>(m_connection, host, port);
		++m_channel_count;

		channel->async_open(yield[ec]);
		if (ec)
		{
			send_reply(socket, zh::reply{zh::internal_server_error}, yield);
			m_proxy.log_error(ec);
			return;
		}

		zh::reply reply(zeep::http::ok);
		boost::asio::streambuf buffer;
		std::ostream out(&buffer);
		out << reply;

		m_proxy.log_request(client, req, req.get_request_line(), reply);
		boost::asio::async_write(socket, buffer, yield[ec]);

		if (ec)
			m_proxy.log_error(ec);
		else
		{
			std::shared_ptr<connect_copy> cc(new connect_copy{std::move(socket), channel, m_open_channel_count});
			cc->start();
		}
	}

	void send_reply(tcp::socket &socket, const zh::reply &reply, boost::asio::yield_context yield)
	{
		boost::asio::streambuf buffer;
		std::ostream out(&buffer);
		out << reply;

		boost::system::error_code ec;
		boost::asio::async_write(socket, buffer, yield[ec]);
		if (ec)
			m_proxy.log_error(ec);
	}

	void handle_proxy_requests(zh::request req, tcp::socket socket, boost::asio::yield_context yield)
	{
		boost::system::error_code ec;
		std::shared_ptr<pinch::forwarding_channel> channel;
		open_channel_counter cnt(m_open_channel_count);

		while (not ec)
		{
			std::regex re("^(?:http://)?(?:([-$_.+!*'(),[:alnum:];?&=]+)(?::([-$_.+!*'(),[:alnum:];?&=]+))?@)?([-[:alnum:].]+)(?::(\\d+))?(/.*)?");
			std::smatch mr;

			std::string uri = req.get_uri();
			if (not std::regex_match(uri, mr, re))
			{
				send_reply(socket, zh::reply{zh::bad_request}, yield);
				return;
			}

			++m_request_count;

			std::string host;
			uint16_t port = 80;

			if (host.empty())
			{
				if (mr[3].matched)
					host = mr[3].str();
				else
					host = "localhost";

				if (mr[4].matched)
					port = std::stoi(mr[4]);
			}
			else
			{
				auto cp = host.find(':');
				if (cp != std::string::npos)
				{
					port = std::stoi(host.substr(cp + 1));
					host.erase(cp, std::string::npos);
				}
			}

			// m_proxy.validate(m_request);

			if (not(channel and channel->forwards_to(host, port)))
				channel.reset(new pinch::forwarding_channel(m_connection, host, port));

			if (not channel->is_open())
			{
				channel->async_open(yield[ec]);
				++m_channel_count;
			}

			boost::asio::streambuf buffer;
			std::ostream out(&buffer);
			out << req;

			boost::asio::async_write(*channel, buffer, yield[ec]);
			if (ec)
				break;

			zeep::http::reply_parser rep_parser;
			boost::tribool r;

			do
			{
				auto buf = buffer.prepare(1024);
				std::size_t n = channel->async_read_some(buf, yield[ec]);
				if (ec)
					break;
				buffer.commit(n);
				r = rep_parser.parse(buffer);
			} while (not ec and boost::indeterminate(r));

			auto reply = rep_parser.get_reply();

			out.clear();
			out << reply;
			boost::asio::async_write(socket, buffer, yield[ec]);
			if (ec)
				break;

			// restart with a next request unless it is HTTP/1.0
			const auto &[major, minor] = req.get_version();
			if (minor == 0)
				break;

			zeep::http::request_parser req_parser;

			do
			{
				auto buf = buffer.prepare(1024);
				std::size_t n = boost::asio::async_read(socket, buf, boost::asio::transfer_at_least(1), yield[ec]);
				if (ec)
					break;
				buffer.commit(n);
				r = req_parser.parse(buffer);
			} while (not ec and boost::indeterminate(r));

			req = req_parser.get_request();
		}

		if (ec)
			m_proxy.log_error(ec);
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
	catch (const boost::system::error_code &ec)
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
	MHTTPServer(MHTTPProxyImpl& proxy, boost::asio::io_context &io_context, zeep::http::security_context *sctxt)
		: zeep::http::basic_server(sctxt)
		, m_io_context(io_context)
		, m_proxy(proxy)
	{
	}

	boost::asio::io_context &get_io_context() override
	{
		return m_io_context;
	}

	void log_request(const std::string &client, const zh::request &req, const zh::reply &rep,
		const boost::posix_time::ptime &start, const std::string &referer,
		const std::string &userAgent, const std::string &entry) noexcept override
	{
		// m_proxy.log_request(client, req, req.get_request_line(), rep);
	}

  private:
	boost::asio::io_context &m_io_context;
	MHTTPProxyImpl& m_proxy;
};

// --------------------------------------------------------------------

MHTTPProxyImpl::MHTTPProxyImpl(std::shared_ptr<pinch::basic_connection> inConnection, uint16_t inPort,
	bool require_authentication, const std::string &user, const std::string &password, log_level log)
	: m_user_service({{user, password, {"PROXY_USER"}}})
	, m_connection(inConnection)
	, m_log_level(log)
{
#if DEBUG
	set_log_level(log_level::debug);
#endif

	// std::string secret = zeep::encode_base64(zeep::random_hash());
	// auto sc = new zeep::http::security_context(secret, m_user_service, not require_authentication);

	// sc->add_rule("/status", "PROXY_USER");
	// sc->add_rule("/", {});

	// m_server.reset(new MHTTPServer(gApp->get_io_context(), sc));
	m_server.reset(new MHTTPServer(*this, gApp->get_io_context(), nullptr));

	m_server->set_allowed_methods({"GET", "POST", "PUT", "OPTIONS", "HEAD", "DELETE", "CONNECT"});

	m_server->add_error_handler(new http_proxy_error_handler());

	m_server->set_template_processor(new zeep::http::rsrc_based_html_template_processor(""));

	// m_server->add_controller(new zeep::http::login_controller());
	m_server->add_controller(new proxy_controller(m_connection, *this));

	m_server->bind("::", inPort);
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
	{
		using namespace boost::local_time;

		m_log.reset(new std::ofstream(gPrefsDir / "proxy.log", std::ios::app));

		local_time_facet *lf(new local_time_facet("[%d/%b/%Y:%H:%M:%S %z]"));
		m_log->imbue(std::locale(std::cout.getloc(), lf));
	}
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

			using namespace boost::local_time;
			local_date_time start_local(request.get_timestamp(), time_zone_ptr());

			*m_log << client << ' '
				   << "-" << ' '
				   << "-" << ' '
				   << start_local << ' '
				   << '"' << request_line << "\" "
				   << std::to_string(reply.get_status()) << ' '
				   << reply.size() << ' '
				   << '"' << referer << '"' << ' '
				   << '"' << userAgent << '"'
				   << std::endl;
		}
	}
	catch (...)
	{
	}
}

void MHTTPProxyImpl::log_error(const std::exception &e)
{
#if DEBUG
	std::cerr << "ERROR (ex): " << e.what() << std::endl;
#endif

	if (m_log)
		*m_log << "ERROR: " << e.what() << std::endl;
}

void MHTTPProxyImpl::log_error(const boost::system::error_code &ec)
{
#if DEBUG
	std::cerr << "ERROR (ec): " << ec.message() << std::endl;
#endif

	if (m_log and
		ec != pinch::error::make_error_code(pinch::error::channel_closed) and
		ec != pinch::error::make_error_code(pinch::error::connection_lost) and
		ec != boost::asio::error::make_error_code(boost::asio::error::eof))
	{
		*m_log << "ERROR: " << ec.message() << std::endl;
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
	auto user = Preferences::GetString("http-proxy-user", "");
	auto password = Preferences::GetString("http-proxy-password", "");
	m_impl = new MHTTPProxyImpl(inConnection, inPort, require_authentication, user, password, log);
}
