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

void decode_base64(
	const string&		inString,
	vector<uint8_t>&		outBinary)
{
    const char kLookupTable[] =
    {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
        -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
        -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1
    };
    
    string::const_iterator b = inString.begin();
    string::const_iterator e = inString.end();
    
	while (b != e)
	{
		uint8_t s[4] = {};
		int n = 0;
		
		for (int i = 0; i < 4 and b != e; ++i)
		{
			uint8_t ix = uint8_t(*b++);

			if (ix == '=')
				break;
			
			char v = -1;
			if (ix <= 127) 
				v = kLookupTable[ix];
			if (v < 0)	THROW(("Invalid character in base64 encoded string"));
			s[i] = uint8_t(v);
			++n;
		}

		if (n > 1)	outBinary.push_back(s[0] << 2 | s[1] >> 4);
		if (n > 2)	outBinary.push_back(s[1] << 4 | s[2] >> 2);
		if (n > 3)	outBinary.push_back(s[2] << 6 | s[3]);
	}
}

void decode_base32(const string& inString, vector<uint8_t>& outBinary)
{
	int buffer = 0;
	int bitsLeft = 0;

	for (uint8_t ch: inString)
	{
		if (ch == ' ' or ch == '\t' or ch == '\r' or ch == '\n' or ch == '-')
			continue;

		buffer <<= 5;

		// Deal with commonly mistyped characters
		if (ch == '0')
			ch = 'O';
		else if (ch == '1')
			ch = 'L';
		else if (ch == '8')
			ch = 'B';

		// Look up one base32 digit
		if ((ch >= 'A' and ch <= 'Z') or (ch >= 'a' and ch <= 'z'))
			ch = (ch & 0x1f) - 1;
		else if (ch >= '2' and ch <= '7')
			ch -= '2' - 26;
		else
			throw runtime_error("invalid character in base32 encoded string");
		
		buffer |= ch;
		bitsLeft += 5;
		if (bitsLeft >= 8)
		{
			outBinary.push_back(buffer >> (bitsLeft - 8));
			bitsLeft -= 8;
		}
	}
}


