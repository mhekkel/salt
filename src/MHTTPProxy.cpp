//           Copyright Maarten L. Hekkelman 2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MSalt.hpp"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/write.hpp>

#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <pinch/port_forwarding.hpp>

#include <zeep/crypto.hpp>
#include <zeep/http/html-controller.hpp>
#include <zeep/http/message-parser.hpp>
#include <zeep/http/security.hpp>
#include <zeep/http/server.hpp>
#include <zeep/http/error-handler.hpp>
#include <zeep/http/login-controller.hpp>

#include "MHTTPProxy.hpp"
#include "MPreferences.hpp"
// #include "mrsrc.hpp"
// #include "MApplication.hpp"

// #if defined(_MSC_VER)

// #define BOOST_LIB_NAME boost_coroutine

// // tell the auto-link code to select a dll when required:
// #if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_WAVE_DYN_LINK)
// #define BOOST_DYN_LINK
// #endif

// #include <boost/config/auto_link.hpp>

// #endif

// using namespace std;
// using namespace pinch;

using tcp = boost::asio::ip::tcp;
namespace zh = zeep::http;
// namespace ba = boost::algorithm;
namespace fs = std::filesystem;
using json = zeep::json::element;

// 	void set_log_flags(uint32_t log_flags);

// 	using zeep::http::server::log_request;

// 	void log_request(const std::string &client,
// 					 const zeep::http::request &req, const std::string &request_line,
// 					 const zeep::http::reply &rep);
// 	void log_error(const std::exception &e);
// 	void log_error(const boost::system::error_code &ec);

// 	void validate(zeep::http::request &request);

// 	std::shared_ptr<pinch::basic_connection> get_connection() const { return m_connection; }

// private:
// 	MHTTPProxy(std::shared_ptr<pinch::basic_connection> connection, bool require_authentication, uint32_t log_flags);

// 	void listen(uint16_t port);
// 	void stop();

// 	virtual void load_template(const std::string &file, zeep::xml::document &doc);
// 	virtual void handle_file(const zeep::http::request &request, const zeep::http::scope &scope, zeep::http::reply &reply);

// 	void handle_accept(const boost::system::error_code &ec);

// 	void welcome(const zeep::http::request &request, const zeep::http::scope &scope, zeep::http::reply &reply);
// 	void status(const zeep::http::request &request, const zeep::http::scope &scope, zeep::http::reply &reply);

// 	static std::shared_ptr<MHTTPProxy> sInstance;

// 	std::shared_ptr<pinch::basic_connection> m_connection;
// 	std::shared_ptr<proxy_controller> m_new_connection;
// 	std::shared_ptr<std::ostream> m_log;
// 	std::shared_ptr<boost::asio::ip::tcp::acceptor> m_acceptor;
// 	uint32_t m_log_flags;
// 	// zeep::http::authentication_validation_base *m_proxy_authentication = nullptr;

using namespace boost::posix_time;

using boost::asio::ip::tcp;
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::redirect_error;
using boost::asio::use_awaitable;

class proxy_controller;

// --------------------------------------------------------------------

class MHTTPProxyImpl
{
  public:
	MHTTPProxyImpl(std::shared_ptr<pinch::basic_connection> inConnection, uint16_t inPort,
		bool require_authentication, const std::string &user, const std::string &password, log_level log);

	~MHTTPProxyImpl();

  private:
	zeep::http::simple_user_service m_user_service;

	std::shared_ptr<pinch::basic_connection> m_connection;
	std::unique_ptr<zeep::http::server> m_server;
	std::thread m_io_thread;
};

// --------------------------------------------------------------------
// A handler for CONNECT requests

class proxy_client_channel_connect : public std::enable_shared_from_this<proxy_client_channel_connect>
{
  public:
	proxy_client_channel_connect(tcp::socket &&socket, std::shared_ptr<pinch::basic_connection> connection,
		const std::string &host, uint16_t port)
		: m_socket(std::move(socket))
		, m_channel(new pinch::forwarding_channel(connection, host, port))
	{
	}

	~proxy_client_channel_connect()
	{
		std::cerr << "channel closed" << std::endl;
	}

	void start()
	{
		m_channel->async_open(std::bind(&proxy_client_channel_connect::handle_open, shared_from_this(), std::placeholders::_1));
	}

  private:
	void handle_open(boost::system::error_code ec);
	void handle_wrote_reply(boost::system::error_code ec, std::size_t);

	template <class SocketFrom, class SocketTo>
	awaitable<void> connect_copy(SocketFrom &from, SocketTo &to, std::shared_ptr<proxy_client_channel_connect> self)
	{
		char data[1024];

		for (;;)
		{
			auto length = co_await boost::asio::async_read(from, boost::asio::buffer(data), boost::asio::transfer_at_least(1), use_awaitable);
			co_await boost::asio::async_write(to, boost::asio::buffer(data, length), use_awaitable);
		}
	}
	tcp::socket m_socket;
	std::shared_ptr<pinch::forwarding_channel> m_channel;
	boost::asio::streambuf m_reply_buffer;
};

void proxy_client_channel_connect::handle_open(boost::system::error_code ec)
{
	std::string client;

	try // asking for the remote endpoint address failed sometimes
		// causing aborting exceptions, so I moved it here.
	{
		boost::asio::ip::address addr = m_socket.remote_endpoint().address();
		client = addr.to_string();
	}
	catch (...)
	{
		client = "unknown";
	}

	// m_proxy->log_request(client, m_request, m_request_line, reply);

	zh::reply reply(zeep::http::ok);

	std::ostream out(&m_reply_buffer);
	out << reply;

	(void)boost::asio::async_write(m_socket, m_reply_buffer,
		std::bind(&proxy_client_channel_connect::handle_wrote_reply, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void proxy_client_channel_connect::handle_wrote_reply(boost::system::error_code ec, std::size_t)
{
	if (ec)
	{
		// m_proxy->log_error(ec);
		m_socket.close();
	}
	else
	{
		co_spawn(m_socket.get_executor(), connect_copy(m_socket, *m_channel, shared_from_this()), detached);
		co_spawn(m_socket.get_executor(), connect_copy(*m_channel, m_socket, shared_from_this()), detached);
	}
}

// --------------------------------------------------------------------

// class proxy_client_channel
// {
//   public:
// 	proxy_client_channel(tcp::socket&& socket, std::shared_ptr<pinch::connection> connection,
// 		const std::string& host, uint16_t port)
// 		: m_socket(std::move(socket))
// 		, m_channel(new pinch::forwarding_channel(m_connection, host, port))
	
// 	{
// 		// m_channel.reset(new pinch::forwarding_channel());
// 		// m_channel->async_open(
// 		// 	std::bind(&proxy_controller::handle_open_channel, shared_from_this(), boost::asio::placeholders::error, true));

// 	}

//   private:
// 	tcp::socket m_socket;
// 	std::shared_ptr<pinch::forwarding_channel> m_channel;

// };
/*
struct async_proxy_connect_impl
{
	tcp::socket m_socket;
	std::string m_host;
	uint16_t m_port;
	std::shared_ptr<pinch::forwarding_channel> m_channel;
	zh::request m_request;
	std::shared_ptr<boost::asio::streambuf> m_reply_buffer;

	std::shared_ptr<boost::asio::streambuf> m_buffer = new boost::asio::streambuf();

	enum { start, opening_channel, read, write } m_state = start;

	template<typename Self>
	void operator()(Self &self, boost::system::error_code ec = {}, std::size_t bytes_transferred = 0);

};

template<typename Self>
void async_proxy_connect_impl::operator()(Self &self, boost::system::error_code ec, std::size_t bytes_transferred)
{
	if (not ec)
	{
		switch (m_state)
		{
			case start:
				m_state = opening_channel;
				m_channel->async_open(std::move(self));
				return;
			
			case opening_channel:
				// write the request to the server
				iostream out(m_buffer.get());
				out << m_request;

				m_state = write;
				boost::asio::async_write(*m_channel, *m_buffer, std::move(this));
				return;
			
			case write:
				m_state = read;
				boost::asio::async_read(*m_channel, *m_reply_buffer, boost::asio::transfer_at_least(1), std::move(self));
				return;




		}
	}

	self.complete(ec);
}
*/

// --------------------------------------------------------------------

class proxy_controller : public zeep::http::html_controller, public std::enable_shared_from_this<proxy_controller>
{
  public:
	proxy_controller(std::shared_ptr<pinch::basic_connection> ssh_connection)
		: m_connection(ssh_connection)
	{
		mount_get("status", &proxy_controller::handle_status);
		mount_get("css/", &proxy_controller::handle_file);
	}

	~proxy_controller()
	{
		// if (m_channel)
		// 	m_channel->close();
		// --s_connection_count;
	}


	bool dispatch_request(tcp::socket& socket, zh::request& req, zh::reply& reply) override
	{
		m_socket = &socket;
		return zh::html_controller::dispatch_request(socket, req, reply);
	}

	void handle_status(const zeep::http::request& req, const zeep::http::scope& scope, zeep::http::reply& rep)
    {
		// put the http headers in the scope

		zh::scope sub(scope);
		json headers;
		for (const auto& h : req.get_headers())
		{
			headers.push_back({
				{ "name", h.name },
				{ "value", h.value }
			});
		}
		sub.put("headers", headers);

		json stats;

		//zh::object channelcount;
		//channelcount["name"] = "Channel Count";
		//channelcount["value"] = proxy_channel::channel_count();
		//stats.push_back(channelcount);

		//zh::object openchannelcount;
		//openchannelcount["name"] = "Open Channel Count";
		//openchannelcount["value"] = proxy_channel::open_channel_count();
		//stats.push_back(openchannelcount);

		// zh::object connectioncount;
		// connectioncount["name"] = "Connection Count";
		// connectioncount["value"] = proxy_controller::connection_count();
		// stats.push_back(connectioncount);

		sub.put("stats", stats);

        get_template_processor().create_reply_from_template("templates/status.html", sub, rep);
    }

	bool handle_request(zh::request &req, zh::reply &reply) override
	{
		if (req.get_method() == "CONNECT")
		{
			handle_connect(req);
			return true;
		}

		if (req.get_host() == "proxy.hekkelman.net")
			return zh::html_controller::handle_request(req, reply);

		// if (req.get_method() == "OPTIONS" or
		// 	req.get_method() == "HEAD" or
		// 	req.get_method() == "POST" or
		// 	req.get_method() == "GET" or
		// 	req.get_method() == "PUT" or
		// 	req.get_method() == "DELETE" or
		// 	req.get_method() == "TRACE")
		// {
		// }

		reply.set_status(zeep::http::status_type::bad_request);
		reply.set_header("Allow", "CONNECT, OPTIONS, HEAD, POST, GET, PUT, DELETE, TRACE");
		return true;
	}

	void handle_connect(zh::request &req)
	{
		std::string host = req.get_uri();
		uint16_t port = 443;

		auto cp = host.find(':');
		if (cp != std::string::npos)
		{
			port = std::stoi(host.substr(cp + 1));
			host.erase(cp, std::string::npos);
		}

		auto p = std::make_shared<proxy_client_channel_connect>(std::move(*m_socket), m_connection, host, port);
		p->start();
	}

  private:
	std::shared_ptr<pinch::basic_connection> m_connection;
	tcp::socket* m_socket = nullptr;
};



// 	void start();

// 	boost::asio::ip::tcp::socket &get_socket() { return m_socket; }
// 	static uint32_t connection_count() { return s_connection_count; }

//   private:
// 	enum class reply_flag
// 	{
// 		none,
// 		part,
// 		last_part,
// 		unauth,
// 		error,
// 		connect
// 	};

// 	void handle_connect();
// 	void handle_request();

// 	void handle_read_request(const boost::system::error_code &ec);
// 	void handle_open_channel(const boost::system::error_code &ec, bool connect);
// 	void handle_wrote_request(const boost::system::error_code &ec, std::shared_ptr<boost::asio::streambuf>);
// 	void handle_read_reply_header(const boost::system::error_code &ec);
// 	void handle_read_reply_content(const boost::system::error_code &ec);
// 	void handle_wrote_reply(const boost::system::error_code &ec, std::shared_ptr<boost::asio::streambuf>, reply_flag rf);

// 	void handle_error(const boost::system::error_code &ec, zeep::http::status_type err);
// 	void send_error_reply(zh::status_type err);
// 	void send_reply(zh::reply &reply, reply_flag rf);

// 	template <class SocketFrom, class SocketTo>
// 	void connect_copy(SocketFrom &from, SocketTo &to, boost::asio::yield_context yield)
// 	{
// 		char data[1024];
// 		boost::system::error_code ec;

// 		while (not ec)
// 		{
// 			size_t length = boost::asio::async_read(from, boost::asio::buffer(data), boost::asio::transfer_at_least(1), yield[ec]);
// 			if (not ec)
// 				boost::asio::async_write(to, boost::asio::buffer(data, length), yield[ec]);
// 		}
// 	}

// 	// std::shared_ptr<MHTTPProxy> m_proxy;

// 	boost::asio::streambuf m_request_buffer;
// 	zh::request_parser m_request_parser;
// 	// zh::request m_request;
// 	std::string m_request_line;

// 	boost::asio::streambuf m_reply_buffer;
// 	// zh::reply m_reply;
// 	zh::reply_parser m_reply_parser;

// 	// std::shared_ptr<pinch::forwarding_channel> m_channel;
// 	tcp::socket m_socket;
// 	// boost::asio::io_context::strand m_strand;

// 	static uint32_t s_connection_count;

// uint32_t proxy_controller::s_connection_count;

// void proxy_controller::start()
// {
// 	++s_connection_count;
// 	boost::asio::async_read(m_socket, m_request_buffer, boost::asio::transfer_at_least(1),
// 		std::bind(&proxy_controller::handle_read_request, shared_from_this(), boost::asio::placeholders::error));
// }

// void proxy_controller::handle_read_request(const boost::system::error_code &ec)
// {
// 	if (ec)
// 	{
// 		// m_proxy->log_error(ec);
// 		m_socket.close();
// 	}
// 	else if (m_request_buffer.in_avail() == 0)
// 	{
// 		boost::asio::async_read(m_socket, m_request_buffer, boost::asio::transfer_at_least(1),
// 			std::bind(&proxy_controller::handle_read_request, shared_from_this(), boost::asio::placeholders::error));
// 	}
// 	else
// 	{
// 		try
// 		{
// 			boost::tribool result = m_request_parser.parse(m_request_buffer);

// 			if (not result) // error in parser
// 				send_error_reply(zh::bad_request);
// 			else if (result)
// 			{
// 				m_request_parser.reset();

// 				auto request = m_request_parser.get_request();

// 				// save the original request line before it is rewritten by the proxy for logging purposes
// 				const auto &[major, minor] = request.get_version();
// 				m_request_line = request.get_method() + request.get_uri() + "HTTP/" + std::to_string(major) + '.' + std::to_string(minor);

// 				if (request.get_method() == "CONNECT")
// 				{
// 					// m_proxy->validate(request);
// 					handle_connect();
// 				}
// 				else if (request.get_method() == "OPTIONS" or
// 						 request.get_method() == "HEAD" or
// 						 request.get_method() == "POST" or
// 						 request.get_method() == "GET" or
// 						 request.get_method() == "PUT" or
// 						 request.get_method() == "DELETE" or
// 						 request.get_method() == "TRACE")
// 				{
// 					handle_request();
// 				}
// 				else
// 				{
// 					zh::reply reply;
// 					// m_proxy->create_error_reply(m_request, zh::method_not_allowed, "Invalid method in request", reply);
// 					reply.set_header("Allow", "CONNECT, OPTIONS, HEAD, POST, GET, PUT, DELETE, TRACE");
// 					send_reply(reply, reply_flag::error);
// 				}
// 			}
// 			else
// 				boost::asio::async_read(m_socket, m_request_buffer, boost::asio::transfer_at_least(1),
// 					std::bind(&proxy_controller::handle_read_request, shared_from_this(), boost::asio::placeholders::error));
// 		}
// 		catch (zh::authorization_stale_exception &e)
// 		{
// 			zh::reply reply;
// 			m_proxy->create_unauth_reply(m_request, true, kSaltProxyRealm, reply);
// 			reply.set_status(zh::proxy_authentication_required);

// 			send_reply(reply, rf_unauth);
// 		}
// 		catch (zh::unauthorized_exception &e)
// 		{
// 			zh::reply reply;
// 			m_proxy->create_unauth_reply(m_request, false, kSaltProxyRealm, reply);
// 			reply.set_status(zh::proxy_authentication_required);

// 			send_reply(reply, rf_unauth);
// 		}
// 		catch (boost::system::system_error &e)
// 		{
// 			handle_error(e.code(), zh::internal_server_error);
// 		}
// 		catch (exception &e)
// 		{
// 			m_proxy->log_error(e);
// 			m_socket.close();
// 		}
// 	}
// }

// void proxy_controller::handle_connect(zh::request &req)
// {
// 	std::string host = request.uri;
// 	uint16_t port = 443;

// 	string::size_type cp = host.find(':');
// 	if (cp != string::npos)
// 	{
// 		port = std::to_string(host.substr(cp + 1));
// 		host.erase(cp, string::npos);
// 	}

// 	if (m_channel)
// 		m_channel->close();
// 	m_channel.reset(new forwarding_channel(m_proxy->get_connection(), host, port));
// 	m_channel->async_open(std::bind(&proxy_controller::handle_open_channel, shared_from_this(), boost::asio::placeholders::error, true));
// }

// void proxy_controller::handle_request()
// {
// 	string host = m_request.get_header("Host");
// 	uint16_t port = 80;

// 	std::regex re("^(?:http://)?(?:([-$_.+!*'(),[:alnum:];?&=]+)(?::([-$_.+!*'(),[:alnum:];?&=]+))?@)?([-[:alnum:].]+)(?::(\\d+))?(/.*)?");
// 	std::smatch mr;

// 	if (not std::regex_match(m_request.uri, mr, re))
// 		send_error_reply(zh::bad_request);
// 	else
// 	{
// 		if (host.empty())
// 		{
// 			if (mr[3].matched)
// 				host = mr[3];
// 			else
// 				host = "localhost";

// 			if (mr[4].matched)
// 				port = std::to_string(mr[4]);
// 		}
// 		else
// 		{
// 			string::size_type cp = host.find(':');
// 			if (cp != string::npos)
// 			{
// 				port = std::to_string(host.substr(cp + 1));
// 				host.erase(cp, string::npos);
// 			}
// 		}

// 		if (host == "proxy.hekkelman.net" and port == 80)
// 		{
// 			zh::reply reply;
// 			m_proxy->handle_request(m_request, reply);
// 			send_reply(reply, rf_last_part);
// 		}
// 		else
// 		{
// 			// drop the username and password... is that OK?
// 			m_request.uri = mr[5];

// 			m_proxy->validate(m_request);

// 			if (m_channel and m_channel->forwards_to(host, port)) // reuse the open channel
// 				handle_open_channel(boost::system::error_code(), false);
// 			else
// 			{
// 				if (m_channel)
// 					m_channel->close();
// 				m_channel.reset(new forwarding_channel(m_proxy->get_connection(), host, port));
// 				m_channel->async_open(std::bind(&proxy_controller::handle_open_channel, shared_from_this(), boost::asio::placeholders::error, false));
// 			}
// 		}
// 	}
// }

// void proxy_controller::handle_open_channel(const boost::system::error_code &ec, bool connect)
// {
// 	if (ec)
// 		handle_error(ec, zh::bad_gateway);
// 	else if (connect)
// 	{
// 		zh::reply ok;
// 		ok.set_status(zh::ok);

// 		send_reply(ok, rf_connect);
// 	}
// 	else
// 	{
// 		// write the request to the server
// 		shared_ptr<boost::asio::streambuf> buffer(new boost::asio::streambuf);
// 		iostream out(buffer.get());
// 		out << m_request;

// 		boost::asio::async_write(*m_channel, *buffer, std::bind(&proxy_controller::handle_wrote_request, shared_from_this(), boost::asio::placeholders::error, buffer));
// 	}
// }

// void proxy_controller::handle_wrote_request(const boost::system::error_code &ec, shared_ptr<boost::asio::streambuf>)
// {
// 	if (ec)
// 		handle_error(ec, zh::bad_gateway);
// 	else
// 	{
// 		m_reply_parser.reset();

// 		boost::asio::async_read(*m_channel, m_reply_buffer, boost::asio::transfer_at_least(1),
// 			std::bind(&proxy_controller::handle_read_reply_header, shared_from_this(), boost::asio::placeholders::error));
// 	}
// }

// void proxy_controller::handle_read_reply_header(const boost::system::error_code &ec)
// {
// 	if (ec)
// 		handle_error(ec, zh::bad_gateway);
// 	else if (m_reply_buffer.in_avail() == 0)
// 	{
// 		boost::asio::async_read(*m_channel, m_reply_buffer, boost::asio::transfer_at_least(1),
// 			std::bind(&proxy_controller::handle_read_reply_header, shared_from_this(), boost::asio::placeholders::error));
// 	}
// 	else
// 	{
// 		boost::tribool result = m_reply_parser.parse_header(m_reply, m_reply_buffer);

// 		if (result)
// 			send_reply(m_reply, m_reply_parser.parsing_content() ? rf_part : rf_last_part);
// 		else if (not result)
// 			send_error_reply(zh::bad_gateway);
// 		else
// 			boost::asio::async_read(*m_channel, m_reply_buffer, boost::asio::transfer_at_least(1),
// 				std::bind(&proxy_controller::handle_read_reply_header, shared_from_this(), boost::asio::placeholders::error));
// 	}
// }

// void proxy_controller::handle_read_reply_content(const boost::system::error_code &ec)
// {
// 	if (ec)
// 	{
// 		m_socket.close();
// 		m_channel->close();
// 	}
// 	else if (m_reply_buffer.in_avail() == 0)
// 	{
// 		boost::asio::async_read(*m_channel, m_reply_buffer, boost::asio::transfer_at_least(1),
// 			std::bind(&proxy_controller::handle_read_reply_content, shared_from_this(), boost::asio::placeholders::error));
// 	}
// 	else
// 	{
// 		shared_ptr<boost::asio::streambuf> sink(new boost::asio::streambuf);

// 		boost::tribool result = m_reply_parser.parse_content(m_reply, m_reply_buffer, *sink);

// 		boost::asio::async_write(m_socket, *sink,
// 			std::bind(&proxy_controller::handle_wrote_reply, shared_from_this(), boost::asio::placeholders::error, sink, result ? rf_last_part : rf_part));
// 	}
// }

// void proxy_controller::handle_wrote_reply(const boost::system::error_code &ec, shared_ptr<boost::asio::streambuf>, reply_flag rf)
// {
// 	if (ec)
// 	{
// 		m_proxy->log_error(ec);
// 		m_socket.close();
// 	}
// 	else
// 	{
// 		shared_ptr<proxy_controller> self(shared_from_this());

// 		switch (rf)
// 		{
// 			case rf_part:
// 				handle_read_reply_content(ec);
// 				break;
// 			case rf_connect:
// 				boost::asio::spawn(m_strand, [self](boost::asio::yield_context yield) { self->connect_copy(self->m_socket, *self->m_channel, yield); });
// 				boost::asio::spawn(m_strand, [self](boost::asio::yield_context yield) { self->connect_copy(*self->m_channel, self->m_socket, yield); });
// 				break;
// 			default:
// 				if (m_request.keep_alive() and m_reply.keep_alive())
// 					handle_read_request(ec);
// 				break;
// 		}
// 	}
// }

// void proxy_controller::handle_error(const boost::system::error_code &ec, zh::status_type err)
// {
// 	m_proxy->log_error(ec);

// 	zh::reply reply;
// 	m_proxy->create_error_reply(m_request, err, ec.message(), reply);
// 	send_reply(reply, rf_error);
// }

// void proxy_controller::send_error_reply(zh::status_type err)
// {
// 	zh::reply reply;
// 	m_proxy->create_error_reply(m_request, err, "", reply);
// 	send_reply(reply, rf_error);
// }

// void proxy_controller::send_reply(zh::reply &reply, reply_flag rf)
// {
// 	string client;

// 	try // asking for the remote endpoint address failed sometimes
// 		// causing aborting exceptions, so I moved it here.
// 	{
// 		boost::asio::ip::address addr = m_socket.remote_endpoint().address();
// 		client = std::to_string(addr);
// 	}
// 	catch (...)
// 	{
// 		client = "unknown";
// 	}

// 	m_proxy->log_request(client, m_request, m_request_line, reply);

// 	shared_ptr<boost::asio::streambuf> buffer(new boost::asio::streambuf);
// 	ostream out(buffer.get());
// 	out << reply;

// 	(void)boost::asio::async_write(m_socket, *buffer,
// 		std::bind(&proxy_controller::handle_wrote_reply, shared_from_this(), boost::asio::placeholders::error, buffer, rf));
// }

// --------------------------------------------------------------------

// shared_ptr<MHTTPProxy> MHTTPProxy::sInstance;
// const string kSaltProxyRealm = "Salt HTTP Proxy";

// void MHTTPProxy::InitializeProxy(basic_connection* inConnection, uint16_t inPort, bool require_authentication, log_level log)
// {
// 	if (sInstance)
// 		sInstance->stop();

// 	sInstance.reset(new MHTTPProxy(inConnection, require_authentication, log));
// 	sInstance->listen(inPort);
// }

// MHTTPProxy::MHTTPProxy(basic_connection* ssh_connection, bool require_authentication, uint32_t log_flags)
// 	: zh::rsrc_based_webapp("http://proxy.hekkelman.com/ml"), m_connection(ssh_connection), m_log_flags(e_log_none)
// {
// 	m_connection->reference();

// 	if (require_authentication)
// 	{
// 		m_proxy_authentication = new zh::simple_digest_authentication_validation(kSaltProxyRealm, {
// 			{ Preferences::GetString("http-proxy-user", ""), Preferences::GetString("http-proxy-password", "") }
// 		});
// 		set_authenticator(m_proxy_authentication, false);
// 	}

// #if DEBUG
// 	set_log_flags(e_log_request | e_log_debug);
// #endif

// //#if DEBUG
// //	set_docroot("C:\\Users\\maarten\\projects\\salt\\Resources\\templates");
// //#endif
// 	mount("", &MHTTPProxy::welcome);
// 	mount("status", &MHTTPProxy::status);
// 	// mount("style.css", &MHTTPProxy::handle_file);

// 	if (log_flags != e_log_none)
// 		set_log_flags(log_flags);
// }

// MHTTPProxy::~MHTTPProxy()
// {
// 	delete m_proxy_authentication;
// 	m_connection->release();
// }

// void MHTTPProxy::welcome(const zeep::http::request& request, const zeep::http::scope& scope, zeep::http::reply& reply)
// {
// 	create_reply_from_template("index.html", scope, reply);
// }

// void MHTTPProxy::validate(zh::request& request)
// {
// 	if (m_proxy_authentication)
// 	{
// 		m_proxy_authentication->validate_authentication(request);
// 		request.remove_header("Proxy-Authorization");
// 	}
// }

// void MHTTPProxy::status(const zeep::http::request& request, const zh::scope& scope, zeep::http::reply& reply)
// {
// 	// put the http headers in the scope

// 	zh::scope sub(scope);
// 	vector<zh::object> headers;
// 	for (const zh::header& h : request.headers)
// 	{
// 		zh::object header;
// 		header["name"] = h.name;
// 		header["value"] = h.value;
// 		headers.push_back(header);
// 	}
// 	sub.put("headers", headers);

// 	vector<zh::object> stats;

// 	//zh::object channelcount;
// 	//channelcount["name"] = "Channel Count";
// 	//channelcount["value"] = proxy_channel::channel_count();
// 	//stats.push_back(channelcount);

// 	//zh::object openchannelcount;
// 	//openchannelcount["name"] = "Open Channel Count";
// 	//openchannelcount["value"] = proxy_channel::open_channel_count();
// 	//stats.push_back(openchannelcount);

// 	zh::object connectioncount;
// 	connectioncount["name"] = "Connection Count";
// 	connectioncount["value"] = proxy_controller::connection_count();
// 	stats.push_back(connectioncount);

// 	sub.put("stats", stats);

// 	create_reply_from_template("status.html", sub, reply);
// }

// void MHTTPProxy::set_log_flags(uint32_t log_flags)
// {
// 	m_log_flags = log_flags;

// 	if (m_log_flags)
// 	{
// 		using namespace boost::local_time;

// 		m_log.reset(new std::ofstream(gPrefsDir / "proxy.log", ios::app));

// 		local_time_facet* lf(new local_time_facet("[%d/%b/%Y:%H:%M:%S %z]"));
// 		m_log->imbue(locale(cout.getloc(), lf));
// 	}
// 	else
// 		m_log.reset();
// }

// void MHTTPProxy::load_template(const std::string& file, zeep::xml::document& doc)
// {
// //#if DEBUG
// //	basic_webapp::load_template(file, doc);
// //#else
// 	mrsrc::rsrc rsrc(string("templates/") + file);
// 	if (not rsrc)
// 		throw runtime_error("missing template");

// 	struct membuf : public std::streambuf
// 	{
// 		membuf(char* dict, size_t length)
// 		{
// 			this->setg(dict, dict, dict + length);
// 		}
// 	} buffer(const_cast<char*>(rsrc.data()), rsrc.size());

// 	std::istream is(&buffer);
// 	is >> doc;
// //#endif
// }

// void MHTTPProxy::handle_file(const zh::request& request, const zh::scope& scope, zh::reply& reply)
// {
// 	using namespace boost::local_time;
// 	using namespace boost::posix_time;

// 	fs::path file = scope["baseuri"].as<string>();
// 	mrsrc::rsrc rsrc(string("templates/") + file.string());
// 	if (not rsrc)
// 		create_error_reply(request, zh::not_found, "The requested file was not found on this 'server'", reply);
// 	else
// 	{
// 		// compare with the date/time of our executable, since we're reading resources :-)
// 		string ifModifiedSince;
// 		for (const zeep::http::header& h : request.headers)
// 		{
// 			if (ba::iequals(h.name, "If-Modified-Since"))
// 			{
// 				local_date_time modifiedSince(local_sec_clock::local_time(time_zone_ptr()));

// 				local_time_input_facet* lif1(new local_time_input_facet("%a, %d %b %Y %H:%M:%S GMT"));

// 				stringstream ss;
// 				ss.imbue(std::locale(std::locale::classic(), lif1));
// 				ss.str(h.value);
// 				ss >> modifiedSince;

// 				local_date_time fileDate(from_time_t(fs::last_write_time(gExecutablePath)), time_zone_ptr());

// 				if (fileDate <= modifiedSince)
// 				{
// 					reply = zeep::http::reply::stock_reply(zeep::http::not_modified);
// 					return;
// 				}

// 				break;
// 			}
// 		}

// 		string data(rsrc.data(), rsrc.size());
// 		string mimetype = "text/plain";

// 		if (file.extension() == ".css")
// 			mimetype = "text/css";
// 		else if (file.extension() == ".js")
// 			mimetype = "text/javascript";
// 		else if (file.extension() == ".png")
// 			mimetype = "image/png";
// 		else if (file.extension() == ".svg")
// 			mimetype = "image/svg+xml";
// 		else if (file.extension() == ".html" or file.extension() == ".htm")
// 			mimetype = "text/html";
// 		else if (file.extension() == ".xml" or file.extension() == ".xsl" or file.extension() == ".xslt")
// 			mimetype = "text/xml";
// 		else if (file.extension() == ".xhtml")
// 			mimetype = "application/xhtml+xml";

// 		reply.set_content(data, mimetype);

// 		local_date_time t(local_sec_clock::local_time(time_zone_ptr()));
// 		local_time_facet* lf(new local_time_facet("%a, %d %b %Y %H:%M:%S GMT"));

// 		stringstream s;
// 		s.imbue(std::locale(std::cout.getloc(), lf));

// 		ptime pt = from_time_t(std::filesystem::last_write_time(gExecutablePath));
// 		local_date_time t2(pt, time_zone_ptr());
// 		s << t2;

// 		reply.set_header("Last-Modified", s.str());
// 	}
// }

// void MHTTPProxy::listen(uint16_t port)
// {
// 	if (m_log)
// 		*m_log << "Starting proxy service" << endl;

// 	string address = "0.0.0.0";

// 	m_acceptor.reset(new boost::asio::ip::tcp::acceptor(m_connection->get_io_service()));
// 	m_new_connection.reset(new proxy_controller(m_connection, shared_from_this()));

// 	boost::asio::ip::tcp::resolver resolver(m_connection->get_io_service());
// 	boost::asio::ip::tcp::resolver::query query(address, std::to_string(port));
// 	boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);

// 	m_acceptor->open(endpoint.protocol());
// 	m_acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
// 	m_acceptor->bind(endpoint);
// 	m_acceptor->listen();
// 	m_acceptor->async_accept(m_new_connection->get_socket(),
// 		std::bind(&MHTTPProxy::handle_accept, this, boost::asio::placeholders::error));
// }

// void MHTTPProxy::stop()
// {
// 	m_acceptor->close();
// }

// void MHTTPProxy::handle_accept(const boost::system::error_code& ec)
// {
// 	if (not ec)
// 	{
// 		m_new_connection->start();
// 		m_new_connection.reset(new proxy_controller(m_connection, shared_from_this()));
// 		m_acceptor->async_accept(m_new_connection->get_socket(),
// 			std::bind(&MHTTPProxy::handle_accept, this, boost::asio::placeholders::error));
// 	}
// }

// void MHTTPProxy::log_request(const string& client,
// 	const zh::request& request, const string& request_line,
// 	const zh::reply& reply)
// {
// 	try
// 	{
// 		if (m_log_flags & e_log_request)
// 		{
// 			string referer = request.get_header("Referer");
// 			if (referer.empty()) referer = "-";

// 			string userAgent = request.get_header("User-Agent");
// 			if (userAgent.empty()) userAgent = "-";

// 			using namespace boost::local_time;
// 			local_date_time start_local(request.timestamp(), time_zone_ptr());

// 			*m_log << client << ' '
// 				 << "-" << ' '
// 				 << "-" << ' '
// 				 << start_local << ' '
// 				 << '"' << request_line << "\" "
// 				 << reply.get_status() << ' '
// 				 << reply.get_size() << ' '
// 				 << '"' << referer << '"' << ' '
// 				 << '"' << userAgent << '"'
// 				 << endl;
// 		}

// 		if (m_log_flags & e_log_debug)
// 		{
// 			request.debug(*m_log);

// 			*m_log << endl;

// 			reply.debug(*m_log);

// 			*m_log << endl;
// 		}
// 	}
// 	catch (...) {}
// }

// void MHTTPProxy::log_error(const std::exception& e)
// {
// #if DEBUG
// 	cerr << "ERROR (ex): " << e.what() << endl;
// #endif

// 	if (m_log)
// 		*m_log << "ERROR: " << e.what() << endl;
// }

// void MHTTPProxy::log_error(const boost::system::error_code& ec)
// {
// #if DEBUG
// 	cerr << "ERROR (ec): " << ec.message() << endl;
// #endif

// 	if (m_log and
// 		ec != pinch::error::make_error_code(pinch::error::channel_closed) and
// 		ec != pinch::error::make_error_code(pinch::error::connection_lost) and
// 		ec != boost::asio::error::make_error_code(boost::asio::error::eof))
// 	{
// 		*m_log << "ERROR: " << ec.message() << endl;
// 	}
// }

// --------------------------------------------------------------------

class http_proxy_error_handler : public zeep::http::error_handler
{
  public:
	http_proxy_error_handler() : error_handler("not-found.html") {}

	virtual bool create_error_reply(const zeep::http::request& req, std::exception_ptr eptr, zeep::http::reply& reply);
};

bool http_proxy_error_handler::create_error_reply(const zeep::http::request& req, std::exception_ptr eptr, zeep::http::reply& reply)
{
	bool result = false;

	try
	{
		if (eptr)
			std::rethrow_exception(eptr);
	}
	catch (const boost::system::error_code& ec)
	{
		result = zeep::http::error_handler::create_error_reply(req, zeep::http::internal_server_error, ec.message(), reply);
	}
	catch (const zeep::http::status_type& s)
	{
		if (s == zeep::http::not_found)
			result = zeep::http::error_handler::create_error_reply(req, s, zeep::http::get_status_description(s), reply);
	}

	return result;
}

// --------------------------------------------------------------------

MHTTPProxyImpl::MHTTPProxyImpl(std::shared_ptr<pinch::basic_connection> inConnection, uint16_t inPort,
	bool require_authentication, const std::string &user, const std::string &password, log_level log)
	: m_user_service({ { user, password, { "PROXY_USER" } } })
	, m_connection(inConnection)
{
	std::string secret = zeep::encode_base64(zeep::random_hash());
	auto sc = new zeep::http::security_context(secret, m_user_service, not require_authentication);

	sc->add_rule("/status", "PROXY_USER");
	sc->add_rule("/", {});

	m_server.reset(new zeep::http::server(sc, ""));

	m_server->set_allowed_methods({ "GET", "POST", "PUT", "OPTIONS", "HEAD", "DELETE", "CONNECT" });

	m_server->add_error_handler(new http_proxy_error_handler());

	m_server->set_template_processor(new zeep::http::rsrc_based_html_template_processor(""));

	m_server->add_controller(new zeep::http::login_controller());
	m_server->add_controller(new proxy_controller(m_connection));

	m_server->bind("localhost", inPort);

	m_io_thread = std::thread([this]() { m_server->run(1); });
}

MHTTPProxyImpl::~MHTTPProxyImpl()
{
	if (m_server)
		m_server->stop();
	
	if (m_io_thread.joinable())
		m_io_thread.join();
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
