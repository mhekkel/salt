//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MWinLib.hpp"

#include "zeep/xml/document.hpp"

#include <map>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>

#include "MResources.hpp"
#include "MError.hpp"
#include "MUtils.hpp"

using namespace std;
using namespace zeep;
namespace io = boost::iostreams;

namespace mrsrc
{

struct rsrc_imp
{
	HRSRC		rsrc;
	HGLOBAL		hmem;
	const char*	data;
	uint32_t		size;
	string		name;
};

class index
{
public:
	rsrc_imp*		load(const string& name);

	static index&	instance();

private:
					index();

	map<string,rsrc_imp>
					m_index;
};

index& index::instance()
{
	static index sInstance;
	return sInstance;
}

index::index()
{
	auto_ptr<rsrc_imp> impl(new rsrc_imp);

	impl->rsrc = ::FindResourceW(nullptr, MAKEINTRESOURCEW(1), L"MRSRCIX");
	if (impl->rsrc == nullptr)
		throw rsrc_not_found_exception();
	
	impl->hmem = ::LoadResource(nullptr, impl->rsrc);
	THROW_IF_NIL(impl->hmem);

	impl->size = ::SizeofResource(nullptr, impl->rsrc);

	impl->data = reinterpret_cast<char*>(::LockResource(impl->hmem));
	THROW_IF_NIL(impl->data);

	io::stream<io::array_source> data(impl->data, impl->size);
	xml::document doc(data);

	for (xml::element* rsrc: doc.find("//rsrc"))
	{
		string name = rsrc->get_attribute("name");
		uint32_t nr = boost::lexical_cast<int>(rsrc->get_attribute("nr"));
		
		rsrc_imp& impl(m_index[name]);

		impl.rsrc = ::FindResourceW(nullptr, MAKEINTRESOURCEW(nr), L"MRSRC");
		if (impl.rsrc == nullptr)
			throw rsrc_not_found_exception();
	
		impl.hmem = ::LoadResource(nullptr, impl.rsrc);
		THROW_IF_NIL(impl.hmem);
		impl.size = ::SizeofResource(nullptr, impl.rsrc);
		impl.data = reinterpret_cast<char*>(::LockResource(impl.hmem));
	}
}

rsrc_imp* index::load(const string& name)
{
	map<string,rsrc_imp>::iterator i = m_index.find(name);

	rsrc_imp* result = nullptr;

	if (i != m_index.end())
		result = &i->second;
	
	return result;
}

rsrc::rsrc()
	: m_impl(nullptr)
{
}

rsrc::rsrc(const rsrc& o)
	: m_impl(o.m_impl)
{
}

rsrc& rsrc::operator=(const rsrc& other)
{
	m_impl = other.m_impl;
	return *this;
}

rsrc::rsrc(const string& name)
	: m_impl(index::instance().load(name + '.' + GetUserLocaleName().substr(0, 2)))
{
	if (m_impl == nullptr)
		m_impl = index::instance().load(name);
}

string rsrc::name() const
{
	return m_impl->name;
}

const char* rsrc::data() const
{
	return m_impl->data;
}
	
unsigned long rsrc::size() const
{
	return m_impl->size;
}

rsrc::operator bool () const
{
	return m_impl != nullptr and m_impl->size > 0;
}

rsrc_list rsrc::children() const
{
	return list<rsrc>();
}

}
