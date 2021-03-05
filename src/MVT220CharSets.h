// Copyright Maarten L. Hekkelman 2011
// All rights reserved

#pragma once

const wchar_t
	//	0		1		2		3		4		5		6		7		8		9		a		b		c		d		e		f
	kUKCharSet[96] = {
		' ',	'!',	'"',	0x00a3,	'$',	'%',	'&',	'\'',	'(',	')',	'*',	'+',	',',	'-',	'.',	'/',
		'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',	'8',	'9',	':',	';',	'<',	'=',	'>',	'?',
		'@',	'A',	'B',	'C',	'D',	'E',	'F',	'G',	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
		'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',	'X',	'Y',	'Z',	'[',	'\\',	']',	'^',	'_',
		'`',	'a',	'b',	'c',	'd',	'e',	'f',	'g',	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
		'p',	'q',	'r',	's',	't',	'u',	'v',	'w',	'x',	'y',	'z',	'{',	'|',	'}',	'~',	0
	},
	kUSCharSet[96] = {
		' ',	'!',	'"',	'#',	'$',	'%',	'&',	'\'',	'(',	')',	'*',	'+',	',',	'-',	'.',	'/',
		'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',	'8',	'9',	':',	';',	'<',	'=',	'>',	'?',
		'@',	'A',	'B',	'C',	'D',	'E',	'F',	'G',	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
		'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',	'X',	'Y',	'Z',	'[',	'\\',	']',	'^',	'_',
		'`',	'a',	'b',	'c',	'd',	'e',	'f',	'g',	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
		'p',	'q',	'r',	's',	't',	'u',	'v',	'w',	'x',	'y',	'z',	'{',	'|',	'}',	'~',	0
	},
	kLineCharSet[96] = {
		' ',	'!',	'"',	'#',	'$',	'%',	'&',	'\'',	'(',	')',	'*',	'+',	',',	'-',	'.',	'/',
		'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',	'8',	'9',	':',	';',	'<',	'=',	'>',	'?',
		'@',	'A',	'B',	'C',	'D',	'E',	'F',	'G',	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
		'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',	'X',	'Y',	'Z',	'[',	'\\',	']',	'^',	' ',
		0x2666,	0x2592,	0x2409,	0x240c,	0x240d,	0x240a,	0x00b0,	0x00b1,	0x2424,	0x240b,	0x2518,	0x2510,	0x250c,	0x2514,	0x253c,	0x00af,
		0x23bb,	0x2500,	0x23bc,	0x23bd,	0x251c,	0x2524,	0x2534,	0x252c,	0x2502,	0x2264,	0x2265,	0x03c0,	0x2260,	0x00a3,	0x00b7,	0
	},
	kNLCharSet[96] = {
		' ',	'!',	'"',	0x00a3,	'$',	'%',	'&',	'\'',	'(',	')',	'*',	'+',	',',	'-',	'.',	'/',
		'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',	'8',	'9',	':',	';',	'<',	'=',	'>',	'?',
		0x00be,	'A',	'B',	'C',	'D',	'E',	'F',	'G',	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
		'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',	'X',	'Y',	'Z',	0x0133,	0x00bd,	'|',	'^',	'_',
		'`',	'a',	'b',	'c',	'd',	'e',	'f',	'g',	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
		'p',	'q',	'r',	's',	't',	'u',	'v',	'w',	'x',	'y',	'z',	0x00a8,	0x0192,	0x00bc,	0x00b4,	0
	},
	kDKCharSet[96] = {
		' ',	'!',	'"',	'#',	'$',	'%',	'&',	'\'',	'(',	')',	'*',	'+',	',',	'-',	'.',	'/',
		'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',	'8',	'9',	':',	';',	'<',	'=',	'>',	'?',
		0x00c4,	'A',	'B',	'C',	'D',	'E',	'F',	'G',	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
		'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',	'X',	'Y',	'Z',	0x00c6,	0x00d8,	0x00c5,	0x00dc,	'_',
		0x00e4,	'a',	'b',	'c',	'd',	'e',	'f',	'g',	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
		'p',	'q',	'r',	's',	't',	'u',	'v',	'w',	'x',	'y',	'z',	0x00e6,	0x00f8,	0x00d5,	0x00fc,	0
	},
	kFICharSet[96] = {
		' ',	'!',	'"',	'#',	'$',	'%',	'&',	'\'',	'(',	')',	'*',	'+',	',',	'-',	'.',	'/',
		'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',	'8',	'9',	':',	';',	'<',	'=',	'>',	'?',
		'@',	'A',	'B',	'C',	'D',	'E',	'F',	'G',	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
		'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',	'X',	'Y',	'Z',	0x00c4,	0x00d6,	0x00c5,	0x00dc,	'_',
		0x00e9,	'a',	'b',	'c',	'd',	'e',	'f',	'g',	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
		'p',	'q',	'r',	's',	't',	'u',	'v',	'w',	'x',	'y',	'z',	0x00e4,	0x00f6,	0x00e5,	0x00fc,	0
	},
	kFRCharSet[96] = {
		' ',	'!',	'"',	0x00a3,	'$',	'%',	'&',	'\'',	'(',	')',	'*',	'+',	',',	'-',	'.',	'/',
		'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',	'8',	'9',	':',	';',	'<',	'=',	'>',	'?',
		0x00e0,	'A',	'B',	'C',	'D',	'E',	'F',	'G',	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
		'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',	'X',	'Y',	'Z',	0x00b0,	0x00e7,	0x00a7,	'^',	'_',
		'`',	'a',	'b',	'c',	'd',	'e',	'f',	'g',	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
		'p',	'q',	'r',	's',	't',	'u',	'v',	'w',	'x',	'y',	'z',	0x00e9,	0x00f9,	0x00e8,	0x00a8,	0
	},
	kCACharSet[96] = {
		' ',	'!',	'"',	'#',	'$',	'%',	'&',	'\'',	'(',	')',	'*',	'+',	',',	'-',	'.',	'/',
		'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',	'8',	'9',	':',	';',	'<',	'=',	'>',	'?',
		0x00e0,	'A',	'B',	'C',	'D',	'E',	'F',	'G',	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
		'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',	'X',	'Y',	'Z',	0x00e2,	0x00e7,	0x00ea,	0x00ee,	'_',
		0x00f4,	'a',	'b',	'c',	'd',	'e',	'f',	'g',	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
		'p',	'q',	'r',	's',	't',	'u',	'v',	'w',	'x',	'y',	'z',	0x00e9,	0x00f9,	0x00e8,	0x00fb,	0
	},
	kDECharSet[96] = {
		' ',	'!',	'"',	'#',	'$',	'%',	'&',	'\'',	'(',	')',	'*',	'+',	',',	'-',	'.',	'/',
		'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',	'8',	'9',	':',	';',	'<',	'=',	'>',	'?',
		0x00a7,	'A',	'B',	'C',	'D',	'E',	'F',	'G',	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
		'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',	'X',	'Y',	'Z',	0x00c4,	0x00d6,	0x00dc,	'^',	'_',
		'`',	'a',	'b',	'c',	'd',	'e',	'f',	'g',	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
		'p',	'q',	'r',	's',	't',	'u',	'v',	'w',	'x',	'y',	'z',	0x00e4,	0x00f6,	0x00fc,	0x00df,	0
	},
	kITCharSet[96] = {
		' ',	'!',	'"',	0x00a3,	'$',	'%',	'&',	'\'',	'(',	')',	'*',	'+',	',',	'-',	'.',	'/',
		'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',	'8',	'9',	':',	';',	'<',	'=',	'>',	'?',
		0x00a7,	'A',	'B',	'C',	'D',	'E',	'F',	'G',	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
		'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',	'X',	'Y',	'Z',	0x00b0,	0x00e7,	0x00e9,	'^',	'_',
		0x00f9,	'a',	'b',	'c',	'd',	'e',	'f',	'g',	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
		'p',	'q',	'r',	's',	't',	'u',	'v',	'w',	'x',	'y',	'z',	0x00e0,	0x00f2,	0x00e8,	0x00ec,	0
	},
	kSPCharSet[96] = {
		' ',	'!',	'"',	0x00a3,	'$',	'%',	'&',	'\'',	'(',	')',	'*',	'+',	',',	'-',	'.',	'/',
		'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',	'8',	'9',	':',	';',	'<',	'=',	'>',	'?',
		0x00a7,	'A',	'B',	'C',	'D',	'E',	'F',	'G',	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
		'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',	'X',	'Y',	'Z',	0x00a1,	0x00d1,	0x00bf,	'^',	'_',
		'`',	'a',	'b',	'c',	'd',	'e',	'f',	'g',	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
		'p',	'q',	'r',	's',	't',	'u',	'v',	'w',	'x',	'y',	'z',	0x00b0,	0x00f1,	0x00e7,	'~',	0
	},
	kSECharSet[96] = {
		' ',	'!',	'"',	'#',	'$',	'%',	'&',	'\'',	'(',	')',	'*',	'+',	',',	'-',	'.',	'/',
		'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',	'8',	'9',	':',	';',	'<',	'=',	'>',	'?',
		0x00c9,	'A',	'B',	'C',	'D',	'E',	'F',	'G',	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
		'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',	'X',	'Y',	'Z',	0x00c4,	0x00d6,	0x00c5,	0x00dc,	'_',
		0x00e9,	'a',	'b',	'c',	'd',	'e',	'f',	'g',	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
		'p',	'q',	'r',	's',	't',	'u',	'v',	'w',	'x',	'y',	'z',	0x00e4,	0x00f6,	0x00e5,	0x00fc,	0
	},
	kCHCharSet[96] = {
		' ',	'!',	'"',	0x00f9,	'$',	'%',	'&',	'\'',	'(',	')',	'*',	'+',	',',	'-',	'.',	'/',
		'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',	'8',	'9',	':',	';',	'<',	'=',	'>',	'?',
		0x00e0,	'A',	'B',	'C',	'D',	'E',	'F',	'G',	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
		'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',	'X',	'Y',	'Z',	0x00e9,	0x00e7,	0x00ea,	0x00ee,	0x00e8,
		0x00f4,	'a',	'b',	'c',	'd',	'e',	'f',	'g',	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
		'p',	'q',	'r',	's',	't',	'u',	'v',	'w',	'x',	'y',	'z',	0x00e4,	0x00f6,	0x00fc,	0x00fb,	0
	},
	kISOLatin1Supplemental[96] = {
		0x00A0,	0x00A1,	0x00A2,	0x00A3,	0x00A4,	0x00A5,	0x00A6,	0x00A7,	0x00A8,	0x00A9,	0x00AA,	0x00AB,	0x00AC,	0x0020,	0x00AE,	0x00AF,
		0x00B0,	0x00B1,	0x00B2,	0x00B3,	0x00B4,	0x00B5,	0x00B6,	0x00B7,	0x00B8,	0x00B9,	0x00BA,	0x00BB,	0x00BC,	0x00BD,	0x00BE,	0x00BF,
		0x00C0,	0x00C1,	0x00C2,	0x00C3,	0x00C4,	0x00C5,	0x00C6,	0x00C7,	0x00C8,	0x00C9,	0x00CA,	0x00CB,	0x00CC,	0x00CD,	0x00CE,	0x00CF,
		0x00D0,	0x00D1,	0x00D2,	0x00D3,	0x00D4,	0x00D5,	0x00D6,	0x00D7,	0x00D8,	0x00D9,	0x00DA,	0x00DB,	0x00DC,	0x00DD,	0x00DE,	0x00DF,
		0x00E0,	0x00E1,	0x00E2,	0x00E3,	0x00E4,	0x00E5,	0x00E6,	0x00E7,	0x00E8,	0x00E9,	0x00EA,	0x00EB,	0x00EC,	0x00ED,	0x00EE,	0x00EF,
		0x00F0,	0x00F1,	0x00F2,	0x00F3,	0x00F4,	0x00F5,	0x00F6,	0x00F7,	0x00F8,	0x00F9,	0x00FA,	0x00FB,	0x00FC,	0x00FD,	0x00FE,	0x00FF
	};
