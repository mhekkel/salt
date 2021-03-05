//           Copyright Maarten L. Hekkelman 2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <assh/config.hpp>
#include <assh/channel.hpp>

namespace assh
{

class terminal_channel : public channel
{
  public:
					terminal_channel(basic_connection* connection);
	
	void			set_environment_variable(const std::string& name,
						const std::string& value);
	
	virtual void	opened();

	template<typename Handler>
	void			open_with_pty(uint32 width, uint32 height,
						const std::string& terminal_type,
						bool forward_agent, bool forward_x11,
						const std::string& ssh_command,
						Handler&& handler)
					{
						m_width = width;
						m_height = height;
						m_terminal_type = terminal_type;
						m_forward_agent = forward_agent;
						m_forward_x11 = forward_x11;
						m_command = ssh_command;
						
						async_open(std::move(handler));
					}

	void			open_with_pty(uint32 width, uint32 height,
						const std::string& terminal_type,
						bool forward_agent, bool forward_x11,
						const std::string& ssh_command);

	void			send_window_resize(uint32 width, uint32 height);	

  protected:
	uint32			m_width, m_height;
	std::string		m_terminal_type;
	std::string		m_command;
	bool			m_forward_agent, m_forward_x11;
	environment		m_env;
};
	
}
