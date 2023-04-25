//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <functional>
#include <string>

// MPrimary is a class used to handle PRIMARY atom selections in X
// (that is, selected text, which you can paste somewhere else with the
// middle mouse button)

class MPrimary
{
  public:
	static MPrimary &Instance();

	bool HasText();
	void GetText(std::string &text);
	void SetText(const std::string &text);
	void SetText(std::function<void(std::string &)> provider);

  private:
	MPrimary();
	~MPrimary();

	MPrimary(const MPrimary &);
	MPrimary &operator=(const MPrimary &);

	struct MPrimaryImpl *mImpl;
};
