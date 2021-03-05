//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MXcbLib.h"

#include "zeep/xml/document.hpp"

#include <map>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>

#include "MResources.h"
#include "MError.h"
#include "MUtils.h"


using namespace std;
using namespace zeep;
namespace io = boost::iostreams;

// The following three variables are generated by the japi resource compiler:

extern const mrsrc::rsrc_imp	gResourceIndex[];
extern const char				gResourceData[];
extern const char				gResourceName[];

namespace mrsrc
{

struct rsrc_imp
{
	unsigned int	m_next;
	unsigned int	m_child;
	unsigned int	m_name;
	unsigned int	m_size;
	unsigned int	m_data;
};

// --------------------------------------------------------------------
// Implementation of mrsrc::rsrc

rsrc::rsrc()
	: m_impl(gResourceIndex)
{
}

rsrc::rsrc(const rsrc_imp* imp)
	: m_impl(imp)
{
}

rsrc::rsrc(const rsrc& other)
	: m_impl(other.m_impl)
{
}

rsrc& rsrc::operator=(const rsrc& other)
{
	m_impl = other.m_impl;
	return *this;
}
	
rsrc::rsrc(const string& path)
{
//	static_assert(sizeof(m_impl->m_next) == 4, "invalid size for unsigned int");
	
	m_impl = gResourceIndex;
	
	const char* lang = getenv("LANG");
	string locale(lang ? lang : "");
	if (not locale.empty())
		locale = string(".") + locale.substr(0, 2);
	
	string p(path);
	
	// would love to use boost functions here, however, the dependencies
	// should be minimal of course.
	while (not p.empty())
	{
		if (m_impl->m_child == 0)	// no children, this is an error
			throw rsrc_not_found_exception();

		m_impl = gResourceIndex + m_impl->m_child;
		
		string::size_type s = p.find('/');
		string name;
		
		if (s != string::npos)
		{
			name = p.substr(0, s);
			p.erase(0, s + 1);
		}
		else
			swap(name, p);
		
		for (;;)
		{
			if (not locale.empty() and name + locale == gResourceName + m_impl->m_name)
				break;
			
			if (name == gResourceName + m_impl->m_name)
				break;
			
			if (m_impl->m_next == 0)
				throw rsrc_not_found_exception();

			m_impl = gResourceIndex + m_impl->m_next;
		}
	}
}

string rsrc::name() const
{
	return gResourceName + m_impl->m_name;
}

const char* rsrc::data() const
{
	return gResourceData + m_impl->m_data;
}
	
unsigned long rsrc::size() const
{
	return m_impl->m_size;
}

rsrc::operator bool () const
{
	return m_impl != NULL and m_impl->m_size > 0;
}

rsrc_list rsrc::children() const
{
	rsrc_list result;
	
	if (m_impl->m_child)
	{
		const rsrc_imp* impl = gResourceIndex + m_impl->m_child;
		result.push_back(rsrc(impl));
		while (impl->m_next)
		{
			impl = gResourceIndex + impl->m_next;
			result.push_back(rsrc(impl));
		}
	}
	
	return result;
}


}
