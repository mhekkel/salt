//           Copyright Maarten L. Hekkelman 2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <assh/config.hpp>

#include <vector>
#include <deque>

//#include <boost/tr1/tuple.hpp>
#include <boost/range.hpp>

#include <cryptopp/integer.h>

#include <assh/channel.hpp>

namespace assh
{

class opacket;
class ipacket;
class ssh_private_key_impl;

// --------------------------------------------------------------------
// A private key is an interface to the PKI system.

class ssh_private_key
{
  public:
						ssh_private_key(ssh_private_key_impl* impl);
//						ssh_private_key(const std::string& hash);
//						ssh_private_key(ipacket& blob);
						~ssh_private_key();

						ssh_private_key(const ssh_private_key& key);
	ssh_private_key&	operator=(const ssh_private_key& key);

	std::vector<uint8>	sign(const std::vector<uint8>& session_id, const opacket& data);

	std::vector<uint8>	get_hash() const;
	std::string			get_comment() const;
	
						operator bool() const							{ return m_impl != nullptr; }
	bool				operator==(const ssh_private_key& key) const;

	friend opacket& operator<<(opacket& p, const ssh_private_key& key);

  protected:
	ssh_private_key_impl*	m_impl;
};	

opacket& operator<<(opacket& p, const ssh_private_key& key);

// --------------------------------------------------------------------
// ssh_agent

class ssh_agent
{
  public:

	static ssh_agent&	instance();
	
	void				process_agent_request(ipacket& in, opacket& out);

	void				update();
	void				register_connection(basic_connection* c);
	void				unregister_connection(basic_connection* c);

	typedef std::vector<ssh_private_key>	ssh_private_key_list;
	typedef ssh_private_key_list::iterator	iterator;

	uint32				size() const				{ return m_private_keys.size(); }
	bool				empty() const				{ return m_private_keys.empty(); }

	iterator			begin()						{ return m_private_keys.begin(); }
	iterator			end()						{ return m_private_keys.end(); }
	
	
	ssh_private_key		get_key(const std::string& hash) const;
	ssh_private_key		get_key(ipacket& blob) const;

	// add a PEM encoded private key
	void				add(const std::string& private_key, const std::string& key_comment,
							std::function<bool(std::string&)> provide_password);

	// for Windows only, expose the private keys via a Pageant compatible window
	void				expose_pageant(bool expose);

  private:

						ssh_agent();
						ssh_agent(const ssh_agent&);
						~ssh_agent();
	ssh_agent&			operator=(const ssh_agent&);

	typedef std::list<basic_connection*> connection_list;

	ssh_private_key_list m_private_keys;
	connection_list		 m_registered_connections;
};

// --------------------------------------------------------------------
// ssh_agent_channel is used for forwarding the ssh-agent over a connection

class ssh_agent_channel : public channel
{
  public:
						ssh_agent_channel(basic_connection* connection);
	virtual				~ssh_agent_channel();
	
	virtual void		opened();
	virtual void		receive_data(const char* data, std::size_t size);

  private:
	ipacket				m_packet;
};

}
