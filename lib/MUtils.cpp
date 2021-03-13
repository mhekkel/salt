//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MLib.hpp"

#include <string>
#include <sstream>
#include <string>
#include <stack>
#include <cmath>

#include "MError.hpp"
#include "MUtils.hpp"

using namespace std;

uint16_t CalculateCRC(const void* inData, uint32_t inLength, uint16_t inCRC)
{
	const uint8_t* p = reinterpret_cast<const uint8_t*>(inData);

	while (inLength-- > 0)
	{
		inCRC = static_cast<uint16_t>(inCRC ^ (static_cast<uint16_t>(*p++) << 8));
		for (uint16_t i = 0; i < 8; i++)
		{
			if (inCRC & 0x8000)
				inCRC = static_cast<uint16_t>((inCRC << 1) ^ 0x1021);
			else
				inCRC = static_cast<uint16_t>(inCRC << 1);
		}
	}

	return inCRC;
}

string Escape(string inString)
{
	string result;
	
	for (string::iterator c = inString.begin(); c != inString.end(); ++c)
	{
		if (*c == '\n')
		{
			result += '\\';
			result += 'n';
		}
		else if (*c == '\t')
		{
			result += '\\';
			result += 't';
		}
		else if (*c == '\\')
		{
			result += '\\';
			result += '\\';
		}
		else
			result += *c;
	}
	
	return result;
}

string Unescape(string inString)
{
	string result;
	
	for (string::iterator c = inString.begin(); c != inString.end(); ++c)
	{
		if (*c == '\\')
		{
			++c;
			if (c != inString.end())
			{
				switch (*c)
				{
					case 'n':
						result += '\n';
						break;
					
					case 't':
						result += '\t';
						break;
					
					default:
						result += *c;
						break;
				}
			}
			else
				result += '\\';
		}
		else
			result += *c;
	}
	
	return result;
}

void HexDump(
	const void*		inBuffer,
	uint32_t			inLength,
	ostream&		outStream)
{
	const char kHex[] = "0123456789abcdef";
	char s[] = "xxxxxxxx  cccc cccc cccc cccc  cccc cccc cccc cccc  |................|";
	const int kHexOffset[] = { 10, 12, 15, 17, 20, 22, 25, 27, 31, 33, 36, 38, 41, 43, 46, 48 };
	const int kAsciiOffset = 53;
	
	const unsigned char* data = reinterpret_cast<const unsigned char*>(inBuffer);
	uint32_t offset = 0;
	
	while (offset < inLength)
	{
		int rr = inLength - offset;
		if (rr > 16)
			rr = 16;
		
		char* t = s + 7;
		long o = offset;
		
		while (t >= s)
		{
			*t-- = kHex[o % 16];
			o /= 16;
		}
		
		for (int i = 0; i < rr; ++i)
		{
			s[kHexOffset[i] + 0] = kHex[data[i] >> 4];
			s[kHexOffset[i] + 1] = kHex[data[i] & 0x0f];
			if (data[i] < 128 and not iscntrl(data[i]) and isprint(data[i]))
				s[kAsciiOffset + i] = data[i];
			else
				s[kAsciiOffset + i] = '.';
		}
		
		for (int i = rr; i < 16; ++i)
		{
			s[kHexOffset[i] + 0] = ' ';
			s[kHexOffset[i] + 1] = ' ';
			s[kAsciiOffset + i] = ' ';
		}
		
		outStream << s << endl;
		
		offset += rr;
		data += rr;
	}
}
