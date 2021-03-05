//           Copyright Maarten L. Hekkelman 2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <pinch/config.hpp>

#include <pinch/port_forwarding.hpp>

#include <zeep/http/request.hpp>
#include <zeep/http/reply.hpp>
#include <zeep/http/webapp.hpp>
#include <zeep/http/el-processing.hpp>

class proxy_connection;
class proxy_channel;

enum log_level
{
	e_log_none =	0,
	e_log_request =	1 << 0,
	e_log_debug =	1 << 1
};

extern const std::string kSaltProxyRealm;

class MHTTPProxy : public std::enable_shared_from_this<MHTTPProxy>
				 , public zeep::http::rsrc_based_webapp
{
  public:

	~MHTTPProxy();

	static void InitializeProxy(pinch::basic_connection* inConnection, uint16 inPort, bool require_authentication, log_level log);

	void set_log_flags(uint32 log_flags);

	using zeep::http::server::log_request;

	void log_request(const std::string& client,
		const zeep::http::request& req, const std::string& request_line,
		const zeep::http::reply& rep);
	void log_error(const std::exception& e);
	void log_error(const boost::system::error_code& ec);

	void validate(zeep::http::request& request);

	pinch::basic_connection* get_connection() const { return m_connection; }

  private:

	MHTTPProxy(pinch::basic_connection* connection, bool require_authentication, uint32 log_flags);
	
	void listen(uint16 port);
	void stop();

	virtual void load_template(const std::string& file, zeep::xml::document& doc);
	virtual void handle_file(const zeep::http::request& request, const zeep::http::scope& scope, zeep::http::reply& reply);

	void handle_accept(const boost::system::error_code& ec);

	void welcome(const zeep::http::request& request, const zeep::http::scope& scope, zeep::http::reply& reply);
	void status(const zeep::http::request& request, const zeep::http::scope& scope, zeep::http::reply& reply);

	static std::shared_ptr<MHTTPProxy> sInstance;

	pinch::basic_connection* m_connection;
	std::shared_ptr<proxy_connection> m_new_connection;
	std::shared_ptr<std::ostream> m_log;
	std::shared_ptr<boost::asio::ip::tcp::acceptor> m_acceptor;
	uint32 m_log_flags;
	zeep::http::authentication_validation_base* m_proxy_authentication = nullptr;
};
