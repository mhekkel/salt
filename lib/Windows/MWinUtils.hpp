//          Copyright Maarten L. Hekkelman 2006-2010
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string>

#undef GetUserName

std::wstring c2w(const std::string& s);
std::string w2c(const std::wstring& s);

void LogWinMsg(const char* inWhere, uint32_t inMsg);

template<typename T>
class MComPtr
{
public:
			MComPtr()
				: mPtr(nullptr) {}

			MComPtr(const MComPtr& rhs)
				: mPtr(rhs.mPtr)
			{
				mPtr->AddRef();
			}

	template<class C>
	MComPtr&
			operator=(const MComPtr<C>& rhs)
			{
				if ((const void*)this != (const void*)&rhs)
				{
					if (mPtr != nullptr)
						mPtr->Release();
					
					mPtr = rhs.get();
					
					if (mPtr != nullptr)
						mPtr->AddRef();
				}

				return *this;
			}

			~MComPtr()
			{
				if (mPtr != nullptr)
					mPtr->Release();
			}

			operator T*() const
			{
				return mPtr;
			}

	T*		operator->() const
			{
				return mPtr;
			}

	T**		operator &()
			{
				if (mPtr != nullptr)
					mPtr->Release();
				
				mPtr = nullptr;
				return &mPtr;
			}

	void	reset(T* inPtr)
			{
				if (mPtr != nullptr)
					mPtr->Release();

				mPtr = inPtr;
			}

	T*		get() const
			{
				return mPtr;
			}

	T*		release()
			{
				T* result = mPtr;
				mPtr = nullptr;
				return result;
			}

			operator bool() const
			{
				return mPtr != nullptr;
			}

private:
	T*		mPtr;
};
