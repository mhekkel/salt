//           Copyright Maarten L. Hekkelman 2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <pinch/port_forwarding.hpp>

class proxy_connection;
class proxy_channel;

const std::string kSaltProxyRealm;

enum class log_level
{
	none = 0,
	request = 1 << 0,
	debug = 1 << 1
};

class MHTTPProxy
{
public:
	MHTTPProxy(const MHTTPProxy&) = delete;
	MHTTPProxy& operator=(const MHTTPProxy&) = delete;

	~MHTTPProxy();

	static MHTTPProxy& instance();

	void Init(std::shared_ptr<pinch::basic_connection> inConnection,
		uint16_t inPort, bool require_authentication, log_level log);

private:
	MHTTPProxy();

	struct MHTTPProxyImpl* m_impl;
};
