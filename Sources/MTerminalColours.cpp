// Copyright Maarten L. Hekkelman 2017
// All rights reserved

#include "MSalt.h"

#include "MTerminalColours.h"

const MColor k256AnsiColors[256] = {

	{ uint8(0), uint8(0), uint8(0) },
	{ uint8(147), uint8(29), uint8(29) },
	{ uint8(29), uint8(147), uint8(29) },
	{ uint8(147), uint8(147), uint8(29) },
	{ uint8(29), uint8(49), uint8(147) },
	{ uint8(147), uint8(29), uint8(147) },
	{ uint8(29), uint8(147), uint8(147) },
	{ uint8(183), uint8(183), uint8(183) },
	
	{ uint8(128), uint8(128), uint8(128) },
	{ uint8(204), uint8(80), uint8(80) },
	{ uint8(80), uint8(204), uint8(80) },
	{ uint8(204), uint8(204), uint8(80) },
	{ uint8(80), uint8(80), uint8(204) },
	{ uint8(204), uint8(80), uint8(204) },
	{ uint8(40), uint8(204), uint8(204) },
	{ uint8(209), uint8(209), uint8(209) },

//	// normal colours
//
//	{ uint8(46), uint8(52), uint8(54) },	// 0: #000000
//	{ uint8(204), uint8(0), uint8(0) },		// 1: #800000
//	{ uint8(78), uint8(154), uint8(6) },	// 2: #008000
//	{ uint8(128), uint8(128), uint8(0) },	// 3: #808000
//	{ uint8(0), uint8(0), uint8(128) },		// 4: #000080
//	{ uint8(128), uint8(0), uint8(128) },	// 5: #800080
//	{ uint8(0), uint8(128), uint8(128) },	// 6: #008080
//	{ uint8(192), uint8(192), uint8(192) },	// 7: #c0c0c0
//	
//	// bold colours	
//
//	{ uint8(128), uint8(128), uint8(128) },	// 0: #808080
//	{ uint8(255), uint8(0), uint8(0) },		// 1: #ff0000
//	{ uint8(0), uint8(255), uint8(0) },		// 2: #00ff00
//	{ uint8(255), uint8(255), uint8(0) },	// 3: #ffff00
//	{ uint8(0), uint8(0), uint8(255) },		// 4: #0000ff
//	{ uint8(255), uint8(0), uint8(255) },	// 5: #ff00ff
//	{ uint8(0), uint8(255), uint8(255) },	// 6: #00ffff
//	{ uint8(255), uint8(255), uint8(255) },	// 7: #ffffff
	
	// xterm colours

	{ uint8(0), uint8(0), uint8(0) },		// 16: #000000
	{ uint8(0), uint8(0), uint8(95) },		// 17: #00005f
	{ uint8(0), uint8(0), uint8(135) },		// 18: #000087
	{ uint8(0), uint8(0), uint8(175) },		// 19: #0000af
	{ uint8(0), uint8(0), uint8(215) },		// 20: #0000d7
	{ uint8(0), uint8(0), uint8(255) },		// 21: #0000ff
	{ uint8(0), uint8(95), uint8(0) },		// 22: #005f00
	{ uint8(0), uint8(95), uint8(95) },		// 23: #005f5f
	{ uint8(0), uint8(95), uint8(135) },		// 24: #005f87
	{ uint8(0), uint8(95), uint8(175) },		// 25: #005faf
	{ uint8(0), uint8(95), uint8(215) },		// 26: #005fd7
	{ uint8(0), uint8(95), uint8(255) },		// 27: #005fff
	{ uint8(0), uint8(135), uint8(0) },		// 28: #008700
	{ uint8(0), uint8(135), uint8(95) },		// 29: #00875f
	{ uint8(0), uint8(135), uint8(135) },	// 30: #008787
	{ uint8(0), uint8(135), uint8(175) },	// 31: #0087af
	{ uint8(0), uint8(135), uint8(215) },	// 32: #0087d7
	{ uint8(0), uint8(135), uint8(255) },	// 33: #0087ff
	{ uint8(0), uint8(175), uint8(0) },		// 34: #00af00
	{ uint8(0), uint8(175), uint8(95) },		// 35: #00af5f
	{ uint8(0), uint8(175), uint8(135) },	// 36: #00af87
	{ uint8(0), uint8(175), uint8(175) },	// 37: #00afaf
	{ uint8(0), uint8(175), uint8(215) },	// 38: #00afd7
	{ uint8(0), uint8(175), uint8(255) },	// 39: #00afff
	{ uint8(0), uint8(215), uint8(0) },		// 40: #00d700
	{ uint8(0), uint8(215), uint8(95) },		// 41: #00d75f
	{ uint8(0), uint8(215), uint8(135) },	// 42: #00d787
	{ uint8(0), uint8(215), uint8(175) },	// 43: #00d7af
	{ uint8(0), uint8(215), uint8(215) },	// 44: #00d7d7
	{ uint8(0), uint8(215), uint8(255) },	// 45: #00d7ff
	{ uint8(0), uint8(255), uint8(0) },		// 46: #00ff00
	{ uint8(0), uint8(255), uint8(95) },		// 47: #00ff5f
	{ uint8(0), uint8(255), uint8(135) },	// 48: #00ff87
	{ uint8(0), uint8(255), uint8(175) },	// 49: #00ffaf
	{ uint8(0), uint8(255), uint8(215) },	// 50: #00ffd7
	{ uint8(0), uint8(255), uint8(255) },	// 51: #00ffff
	{ uint8(95), uint8(0), uint8(0) },		// 52: #5f0000
	{ uint8(95), uint8(0), uint8(95) },		// 53: #5f005f
	{ uint8(95), uint8(0), uint8(135) },		// 54: #5f0087
	{ uint8(95), uint8(0), uint8(175) },		// 55: #5f00af
	{ uint8(95), uint8(0), uint8(215) },		// 56: #5f00d7
	{ uint8(95), uint8(0), uint8(255) },		// 57: #5f00ff
	{ uint8(95), uint8(95), uint8(0) },		// 58: #5f5f00
	{ uint8(95), uint8(95), uint8(95) },		// 59: #5f5f5f
	{ uint8(95), uint8(95), uint8(135) },	// 60: #5f5f87
	{ uint8(95), uint8(95), uint8(175) },	// 61: #5f5faf
	{ uint8(95), uint8(95), uint8(215) },	// 62: #5f5fd7
	{ uint8(95), uint8(95), uint8(255) },	// 63: #5f5fff
	{ uint8(95), uint8(135), uint8(0) },		// 64: #5f8700
	{ uint8(95), uint8(135), uint8(95) },	// 65: #5f875f
	{ uint8(95), uint8(135), uint8(135) },	// 66: #5f8787
	{ uint8(95), uint8(135), uint8(175) },	// 67: #5f87af
	{ uint8(95), uint8(135), uint8(215) },	// 68: #5f87d7
	{ uint8(95), uint8(135), uint8(255) },	// 69: #5f87ff
	{ uint8(95), uint8(175), uint8(0) },		// 70: #5faf00
	{ uint8(95), uint8(175), uint8(95) },	// 71: #5faf5f
	{ uint8(95), uint8(175), uint8(135) },	// 72: #5faf87
	{ uint8(95), uint8(175), uint8(175) },	// 73: #5fafaf
	{ uint8(95), uint8(175), uint8(215) },	// 74: #5fafd7
	{ uint8(95), uint8(175), uint8(255) },	// 75: #5fafff
	{ uint8(95), uint8(215), uint8(0) },		// 76: #5fd700
	{ uint8(95), uint8(215), uint8(95) },	// 77: #5fd75f
	{ uint8(95), uint8(215), uint8(135) },	// 78: #5fd787
	{ uint8(95), uint8(215), uint8(175) },	// 79: #5fd7af
	{ uint8(95), uint8(215), uint8(215) },	// 80: #5fd7d7
	{ uint8(95), uint8(215), uint8(255) },	// 81: #5fd7ff
	{ uint8(95), uint8(255), uint8(0) },		// 82: #5fff00
	{ uint8(95), uint8(255), uint8(95) },	// 83: #5fff5f
	{ uint8(95), uint8(255), uint8(135) },	// 84: #5fff87
	{ uint8(95), uint8(255), uint8(175) },	// 85: #5fffaf
	{ uint8(95), uint8(255), uint8(215) },	// 86: #5fffd7
	{ uint8(95), uint8(255), uint8(255) },	// 87: #5fffff
	{ uint8(135), uint8(0), uint8(0) },		// 88: #870000
	{ uint8(135), uint8(0), uint8(95) },		// 89: #87005f
	{ uint8(135), uint8(0), uint8(135) },	// 90: #870087
	{ uint8(135), uint8(0), uint8(175) },	// 91: #8700af
	{ uint8(135), uint8(0), uint8(215) },	// 92: #8700d7
	{ uint8(135), uint8(0), uint8(255) },	// 93: #8700ff
	{ uint8(135), uint8(95), uint8(0) },		// 94: #875f00
	{ uint8(135), uint8(95), uint8(95) },	// 95: #875f5f
	{ uint8(135), uint8(95), uint8(135) },	// 96: #875f87
	{ uint8(135), uint8(95), uint8(175) },	// 97: #875faf
	{ uint8(135), uint8(95), uint8(215) },	// 98: #875fd7
	{ uint8(135), uint8(95), uint8(255) },	// 99: #875fff
	{ uint8(135), uint8(135), uint8(0) },	// 100: #878700
	{ uint8(135), uint8(135), uint8(95) },	// 101: #87875f
	{ uint8(135), uint8(135), uint8(135) },	// 102: #878787
	{ uint8(135), uint8(135), uint8(175) },	// 103: #8787af
	{ uint8(135), uint8(135), uint8(215) },	// 104: #8787d7
	{ uint8(135), uint8(135), uint8(255) },	// 105: #8787ff
	{ uint8(135), uint8(175), uint8(0) },	// 106: #87af00
	{ uint8(135), uint8(175), uint8(95) },	// 107: #87af5f
	{ uint8(135), uint8(175), uint8(135) },	// 108: #87af87
	{ uint8(135), uint8(175), uint8(175) },	// 109: #87afaf
	{ uint8(135), uint8(175), uint8(215) },	// 110: #87afd7
	{ uint8(135), uint8(175), uint8(255) },	// 111: #87afff
	{ uint8(135), uint8(215), uint8(0) },	// 112: #87d700
	{ uint8(135), uint8(215), uint8(95) },	// 113: #87d75f
	{ uint8(135), uint8(215), uint8(135) },	// 114: #87d787
	{ uint8(135), uint8(215), uint8(175) },	// 115: #87d7af
	{ uint8(135), uint8(215), uint8(215) },	// 116: #87d7d7
	{ uint8(135), uint8(215), uint8(255) },	// 117: #87d7ff
	{ uint8(135), uint8(255), uint8(0) },	// 118: #87ff00
	{ uint8(135), uint8(255), uint8(95) },	// 119: #87ff5f
	{ uint8(135), uint8(255), uint8(135) },	// 120: #87ff87
	{ uint8(135), uint8(255), uint8(175) },	// 121: #87ffaf
	{ uint8(135), uint8(255), uint8(215) },	// 122: #87ffd7
	{ uint8(135), uint8(255), uint8(255) },	// 123: #87ffff
	{ uint8(175), uint8(0), uint8(0) },		// 124: #af0000
	{ uint8(175), uint8(0), uint8(95) },		// 125: #af005f
	{ uint8(175), uint8(0), uint8(135) },	// 126: #af0087
	{ uint8(175), uint8(0), uint8(175) },	// 127: #af00af
	{ uint8(175), uint8(0), uint8(215) },	// 128: #af00d7
	{ uint8(175), uint8(0), uint8(255) },	// 129: #af00ff
	{ uint8(175), uint8(95), uint8(0) },		// 130: #af5f00
	{ uint8(175), uint8(95), uint8(95) },	// 131: #af5f5f
	{ uint8(175), uint8(95), uint8(135) },	// 132: #af5f87
	{ uint8(175), uint8(95), uint8(175) },	// 133: #af5faf
	{ uint8(175), uint8(95), uint8(215) },	// 134: #af5fd7
	{ uint8(175), uint8(95), uint8(255) },	// 135: #af5fff
	{ uint8(175), uint8(135), uint8(0) },	// 136: #af8700
	{ uint8(175), uint8(135), uint8(95) },	// 137: #af875f
	{ uint8(175), uint8(135), uint8(135) },	// 138: #af8787
	{ uint8(175), uint8(135), uint8(175) },	// 139: #af87af
	{ uint8(175), uint8(135), uint8(215) },	// 140: #af87d7
	{ uint8(175), uint8(135), uint8(255) },	// 141: #af87ff
	{ uint8(175), uint8(175), uint8(0) },	// 142: #afaf00
	{ uint8(175), uint8(175), uint8(95) },	// 143: #afaf5f
	{ uint8(175), uint8(175), uint8(135) },	// 144: #afaf87
	{ uint8(175), uint8(175), uint8(175) },	// 145: #afafaf
	{ uint8(175), uint8(175), uint8(215) },	// 146: #afafd7
	{ uint8(175), uint8(175), uint8(255) },	// 147: #afafff
	{ uint8(175), uint8(215), uint8(0) },	// 148: #afd700
	{ uint8(175), uint8(215), uint8(95) },	// 149: #afd75f
	{ uint8(175), uint8(215), uint8(135) },	// 150: #afd787
	{ uint8(175), uint8(215), uint8(175) },	// 151: #afd7af
	{ uint8(175), uint8(215), uint8(215) },	// 152: #afd7d7
	{ uint8(175), uint8(215), uint8(255) },	// 153: #afd7ff
	{ uint8(175), uint8(255), uint8(0) },	// 154: #afff00
	{ uint8(175), uint8(255), uint8(95) },	// 155: #afff5f
	{ uint8(175), uint8(255), uint8(135) },	// 156: #afff87
	{ uint8(175), uint8(255), uint8(175) },	// 157: #afffaf
	{ uint8(175), uint8(255), uint8(215) },	// 158: #afffd7
	{ uint8(175), uint8(255), uint8(255) },	// 159: #afffff
	{ uint8(215), uint8(0), uint8(0) },		// 160: #d70000
	{ uint8(215), uint8(0), uint8(95) },		// 161: #d7005f
	{ uint8(215), uint8(0), uint8(135) },	// 162: #d70087
	{ uint8(215), uint8(0), uint8(175) },	// 163: #d700af
	{ uint8(215), uint8(0), uint8(215) },	// 164: #d700d7
	{ uint8(215), uint8(0), uint8(255) },	// 165: #d700ff
	{ uint8(215), uint8(95), uint8(0) },		// 166: #d75f00
	{ uint8(215), uint8(95), uint8(95) },	// 167: #d75f5f
	{ uint8(215), uint8(95), uint8(135) },	// 168: #d75f87
	{ uint8(215), uint8(95), uint8(175) },	// 169: #d75faf
	{ uint8(215), uint8(95), uint8(215) },	// 170: #d75fd7
	{ uint8(215), uint8(95), uint8(255) },	// 171: #d75fff
	{ uint8(215), uint8(135), uint8(0) },	// 172: #d78700
	{ uint8(215), uint8(135), uint8(95) },	// 173: #d7875f
	{ uint8(215), uint8(135), uint8(135) },	// 174: #d78787
	{ uint8(215), uint8(135), uint8(175) },	// 175: #d787af
	{ uint8(215), uint8(135), uint8(215) },	// 176: #d787d7
	{ uint8(215), uint8(135), uint8(255) },	// 177: #d787ff
	{ uint8(215), uint8(175), uint8(0) },	// 178: #d7af00
	{ uint8(215), uint8(175), uint8(95) },	// 179: #d7af5f
	{ uint8(215), uint8(175), uint8(135) },	// 180: #d7af87
	{ uint8(215), uint8(175), uint8(175) },	// 181: #d7afaf
	{ uint8(215), uint8(175), uint8(215) },	// 182: #d7afd7
	{ uint8(215), uint8(175), uint8(255) },	// 183: #d7afff
	{ uint8(215), uint8(215), uint8(0) },	// 184: #d7d700
	{ uint8(215), uint8(215), uint8(95) },	// 185: #d7d75f
	{ uint8(215), uint8(215), uint8(135) },	// 186: #d7d787
	{ uint8(215), uint8(215), uint8(175) },	// 187: #d7d7af
	{ uint8(215), uint8(215), uint8(215) },	// 188: #d7d7d7
	{ uint8(215), uint8(215), uint8(255) },	// 189: #d7d7ff
	{ uint8(215), uint8(255), uint8(0) },	// 190: #d7ff00
	{ uint8(215), uint8(255), uint8(95) },	// 191: #d7ff5f
	{ uint8(215), uint8(255), uint8(135) },	// 192: #d7ff87
	{ uint8(215), uint8(255), uint8(175) },	// 193: #d7ffaf
	{ uint8(215), uint8(255), uint8(215) },	// 194: #d7ffd7
	{ uint8(215), uint8(255), uint8(255) },	// 195: #d7ffff
	{ uint8(255), uint8(0), uint8(0) },		// 196: #ff0000
	{ uint8(255), uint8(0), uint8(95) },		// 197: #ff005f
	{ uint8(255), uint8(0), uint8(135) },	// 198: #ff0087
	{ uint8(255), uint8(0), uint8(175) },	// 199: #ff00af
	{ uint8(255), uint8(0), uint8(215) },	// 200: #ff00d7
	{ uint8(255), uint8(0), uint8(255) },	// 201: #ff00ff
	{ uint8(255), uint8(95), uint8(0) },		// 202: #ff5f00
	{ uint8(255), uint8(95), uint8(95) },	// 203: #ff5f5f
	{ uint8(255), uint8(95), uint8(135) },	// 204: #ff5f87
	{ uint8(255), uint8(95), uint8(175) },	// 205: #ff5faf
	{ uint8(255), uint8(95), uint8(215) },	// 206: #ff5fd7
	{ uint8(255), uint8(95), uint8(255) },	// 207: #ff5fff
	{ uint8(255), uint8(135), uint8(0) },	// 208: #ff8700
	{ uint8(255), uint8(135), uint8(95) },	// 209: #ff875f
	{ uint8(255), uint8(135), uint8(135) },	// 210: #ff8787
	{ uint8(255), uint8(135), uint8(175) },	// 211: #ff87af
	{ uint8(255), uint8(135), uint8(215) },	// 212: #ff87d7
	{ uint8(255), uint8(135), uint8(255) },	// 213: #ff87ff
	{ uint8(255), uint8(175), uint8(0) },	// 214: #ffaf00
	{ uint8(255), uint8(175), uint8(95) },	// 215: #ffaf5f
	{ uint8(255), uint8(175), uint8(135) },	// 216: #ffaf87
	{ uint8(255), uint8(175), uint8(175) },	// 217: #ffafaf
	{ uint8(255), uint8(175), uint8(215) },	// 218: #ffafd7
	{ uint8(255), uint8(175), uint8(255) },	// 219: #ffafff
	{ uint8(255), uint8(215), uint8(0) },	// 220: #ffd700
	{ uint8(255), uint8(215), uint8(95) },	// 221: #ffd75f
	{ uint8(255), uint8(215), uint8(135) },	// 222: #ffd787
	{ uint8(255), uint8(215), uint8(175) },	// 223: #ffd7af
	{ uint8(255), uint8(215), uint8(215) },	// 224: #ffd7d7
	{ uint8(255), uint8(215), uint8(255) },	// 225: #ffd7ff
	{ uint8(255), uint8(255), uint8(0) },	// 226: #ffff00
	{ uint8(255), uint8(255), uint8(95) },	// 227: #ffff5f
	{ uint8(255), uint8(255), uint8(135) },	// 228: #ffff87
	{ uint8(255), uint8(255), uint8(175) },	// 229: #ffffaf
	{ uint8(255), uint8(255), uint8(215) },	// 230: #ffffd7
	{ uint8(255), uint8(255), uint8(255) },	// 231: #ffffff

	// gray scale
	{ uint8(8), uint8(8), uint8(8) },		// 232: #080808
	{ uint8(18), uint8(18), uint8(18) },		// 233: #121212
	{ uint8(28), uint8(28), uint8(28) },		// 234: #1c1c1c
	{ uint8(38), uint8(38), uint8(38) },		// 235: #262626
	{ uint8(48), uint8(48), uint8(48) },		// 236: #303030
	{ uint8(58), uint8(58), uint8(58) },		// 237: #3a3a3a
	{ uint8(68), uint8(68), uint8(68) },		// 238: #444444
	{ uint8(78), uint8(78), uint8(78) },		// 239: #4e4e4e
	{ uint8(88), uint8(88), uint8(88) },		// 240: #585858
	{ uint8(98), uint8(98), uint8(98) },		// 241: #626262
	{ uint8(108), uint8(108), uint8(108) },	// 242: #6c6c6c
	{ uint8(118), uint8(118), uint8(118) },	// 243: #767676
	{ uint8(128), uint8(128), uint8(128) },	// 244: #808080
	{ uint8(138), uint8(138), uint8(138) },	// 245: #8a8a8a
	{ uint8(148), uint8(148), uint8(148) },	// 246: #949494
	{ uint8(158), uint8(158), uint8(158) },	// 247: #9e9e9e
	{ uint8(168), uint8(168), uint8(168) },	// 248: #a8a8a8
	{ uint8(178), uint8(178), uint8(178) },	// 249: #b2b2b2
	{ uint8(188), uint8(188), uint8(188) },	// 250: #bcbcbc
	{ uint8(198), uint8(198), uint8(198) },	// 251: #c6c6c6
	{ uint8(208), uint8(208), uint8(208) },	// 252: #d0d0d0
	{ uint8(218), uint8(218), uint8(218) },	// 253: #dadada
	{ uint8(228), uint8(228), uint8(228) },	// 254: #e4e4e4
	{ uint8(238), uint8(238), uint8(238) }	// 255: #eeeeee
};
