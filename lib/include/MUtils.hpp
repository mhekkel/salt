//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <filesystem>

#include "MTypes.hpp"
#include "MColor.hpp"
#include "MFile.hpp"

namespace fs = std::filesystem;

struct no_swapper
{
	template<typename T>
	T			operator()(T inValue) const			{ return inValue; }
};

struct swapper
{
	template<typename T>
	T			operator()(T inValue) const
	{
		this_will_not_compile_I_hope(inValue);
	}
};

template<>
inline
bool swapper::operator()(bool inValue) const
{
	return inValue;
}

template<>
inline
int8_t swapper::operator()(int8_t inValue) const
{
	return inValue;
}

template<>
inline
uint8_t swapper::operator()(uint8_t inValue) const
{
	return inValue;
}

template<>
inline
int16_t swapper::operator()(int16_t inValue) const
{
	return static_cast<int16_t>(((inValue & 0xFF00UL) >>  8) |
		((inValue & 0x00FFUL) <<  8)
	);
}

template<>
inline
uint16_t swapper::operator()(uint16_t inValue) const
{
	return static_cast<uint16_t>(((inValue & 0xFF00UL) >>  8) |
		((inValue & 0x00FFUL) <<  8)
	);
}

template<>
inline
int32_t swapper::operator()(int32_t inValue) const
{
	return static_cast<int32_t>(((inValue & 0xFF000000UL) >> 24) |
		((inValue & 0x00FF0000UL) >>  8) |
		((inValue & 0x0000FF00UL) <<  8) |
		((inValue & 0x000000FFUL) << 24)
	);
}

template<>
inline
uint32_t swapper::operator()(uint32_t inValue) const
{
	return static_cast<uint32_t>(((inValue & 0xFF000000UL) >> 24) |
		((inValue & 0x00FF0000UL) >>  8) |
		((inValue & 0x0000FF00UL) <<  8) |
		((inValue & 0x000000FFUL) << 24)
	);
}

//template<>
//inline
//long unsigned int swapper::operator()(long unsigned int inValue) const
//{
//	return static_cast<long unsigned int>(((inValue & 0xFF000000UL) >> 24) |
//		((inValue & 0x00FF0000UL) >>  8) |
//		((inValue & 0x0000FF00UL) <<  8) |
//		((inValue & 0x000000FFUL) << 24)
//	);
//}

template<>
inline
int64_t swapper::operator()(int64_t inValue) const
{
	return static_cast<int64_t>((((static_cast<uint64_t>(inValue))<<56) & 0xFF00000000000000ULL)  |
		(((static_cast<uint64_t>(inValue))<<40) & 0x00FF000000000000ULL)  |
		(((static_cast<uint64_t>(inValue))<<24) & 0x0000FF0000000000ULL)  |
		(((static_cast<uint64_t>(inValue))<< 8) & 0x000000FF00000000ULL)  |
		(((static_cast<uint64_t>(inValue))>> 8) & 0x00000000FF000000ULL)  |
		(((static_cast<uint64_t>(inValue))>>24) & 0x0000000000FF0000ULL)  |
		(((static_cast<uint64_t>(inValue))>>40) & 0x000000000000FF00ULL)  |
		(((static_cast<uint64_t>(inValue))>>56) & 0x00000000000000FFULL));
}

template<>
inline
uint64_t swapper::operator()(uint64_t inValue) const
{
	return static_cast<uint64_t>(((((uint64_t)inValue)<<56) & 0xFF00000000000000ULL)  |
		((((uint64_t)inValue)<<40) & 0x00FF000000000000ULL)  |
		((((uint64_t)inValue)<<24) & 0x0000FF0000000000ULL)  |
		((((uint64_t)inValue)<< 8) & 0x000000FF00000000ULL)  |
		((((uint64_t)inValue)>> 8) & 0x00000000FF000000ULL)  |
		((((uint64_t)inValue)>>24) & 0x0000000000FF0000ULL)  |
		((((uint64_t)inValue)>>40) & 0x000000000000FF00ULL)  |
		((((uint64_t)inValue)>>56) & 0x00000000000000FFULL));
}

#if BYTE_ORDER == LITTLE_ENDIAN
typedef no_swapper	lsb_swapper;
typedef swapper		msb_swapper;

typedef swapper		net_swapper;
#else
typedef swapper		lsb_swapper;
typedef no_swapper	msb_swapper;

typedef no_swapper	net_swapper;
#endif

// value changer, stack based

template<class T>
class MValueChanger
{
  public:
				MValueChanger(T& inVariable, const T& inNewValue)
					: mVariable(inVariable)
					, mValue(inVariable)
				{
					mVariable = inNewValue;
				}
				
				~MValueChanger()
				{
					mVariable = mValue;
				}
  private:
	T&			mVariable;
	T			mValue;
};

//class MGdkThreadBlock
//{
//  public:
//	MGdkThreadBlock()
//	{
//		gdk_threads_enter();
//	}
//	
//	~MGdkThreadBlock()
//	{
//		gdk_threads_leave();
//	}
//};

uint16_t CalculateCRC(const void* inData, uint32_t inLength, uint16_t inCRC);
std::string Escape(std::string inString);
std::string Unescape(std::string inString);
std::string NumToString(uint32_t inNumber);
uint32_t StringToNum(std::string inString);
std::string	GetUserName(bool inShortName = false);
std::string GetHomeDirectory();
std::string GetPrefsDirectory();
std::string	GetDateTime();
double GetLocalTime();
void delay(double inSeconds);
double GetDblClickTime();
void GetModifierState(uint32_t& outModifiers, bool inAsync);
bool IsModifierDown(int inModifierMask);
void HexDump(const void* inBuffer, uint32_t inLength, std::ostream& outStream);
//GdkPixbuf* CreateDot(// MColor inColor, // uint32_t inSize);
void OpenURI(const std::string& inURI);
std::string GetUserLocaleName();
std::string GetApplicationVersion();

// extern "C" void closefrom(int fd);
