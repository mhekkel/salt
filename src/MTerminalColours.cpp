/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023 Maarten L. Hekkelman
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Copyright Maarten L. Hekkelman 2017
// All rights reserved

#include "MSalt.hpp"

#include "MTerminalColours.hpp"

const MColor k256AnsiColors[256] = {

	{ uint8_t(0), uint8_t(0), uint8_t(0) },
	{ uint8_t(197), uint8_t(0), uint8_t(13) },
	{ uint8_t(0), uint8_t(172), uint8_t(92) },
	{ uint8_t(177), uint8_t(128), uint8_t(30) },
	{ uint8_t(0), uint8_t(63), uint8_t(141) },
	{ uint8_t(157), uint8_t(28), uint8_t(190) },
	{ uint8_t(15), uint8_t(168), uint8_t(192) },
	{ uint8_t(208), uint8_t(207), uint8_t(204) },

	{ uint8_t(94), uint8_t(92), uint8_t(100) },
	{ uint8_t(255), uint8_t(74), uint8_t(54) },
	{ uint8_t(28), uint8_t(223), uint8_t(113) },
	{ uint8_t(244), uint8_t(185), uint8_t(22) },
	{ uint8_t(18), uint8_t(122), uint8_t(249) },
	{ uint8_t(223), uint8_t(63), uint8_t(241) },
	{ uint8_t(32), uint8_t(214), uint8_t(241) },
	{ uint8_t(255), uint8_t(255), uint8_t(255) },

	// xterm colours

	{ uint8_t(0), uint8_t(0), uint8_t(0) },       // 16: #000000
	{ uint8_t(0), uint8_t(0), uint8_t(95) },      // 17: #00005f
	{ uint8_t(0), uint8_t(0), uint8_t(135) },     // 18: #000087
	{ uint8_t(0), uint8_t(0), uint8_t(175) },     // 19: #0000af
	{ uint8_t(0), uint8_t(0), uint8_t(215) },     // 20: #0000d7
	{ uint8_t(0), uint8_t(0), uint8_t(255) },     // 21: #0000ff
	{ uint8_t(0), uint8_t(95), uint8_t(0) },      // 22: #005f00
	{ uint8_t(0), uint8_t(95), uint8_t(95) },     // 23: #005f5f
	{ uint8_t(0), uint8_t(95), uint8_t(135) },    // 24: #005f87
	{ uint8_t(0), uint8_t(95), uint8_t(175) },    // 25: #005faf
	{ uint8_t(0), uint8_t(95), uint8_t(215) },    // 26: #005fd7
	{ uint8_t(0), uint8_t(95), uint8_t(255) },    // 27: #005fff
	{ uint8_t(0), uint8_t(135), uint8_t(0) },     // 28: #008700
	{ uint8_t(0), uint8_t(135), uint8_t(95) },    // 29: #00875f
	{ uint8_t(0), uint8_t(135), uint8_t(135) },   // 30: #008787
	{ uint8_t(0), uint8_t(135), uint8_t(175) },   // 31: #0087af
	{ uint8_t(0), uint8_t(135), uint8_t(215) },   // 32: #0087d7
	{ uint8_t(0), uint8_t(135), uint8_t(255) },   // 33: #0087ff
	{ uint8_t(0), uint8_t(175), uint8_t(0) },     // 34: #00af00
	{ uint8_t(0), uint8_t(175), uint8_t(95) },    // 35: #00af5f
	{ uint8_t(0), uint8_t(175), uint8_t(135) },   // 36: #00af87
	{ uint8_t(0), uint8_t(175), uint8_t(175) },   // 37: #00afaf
	{ uint8_t(0), uint8_t(175), uint8_t(215) },   // 38: #00afd7
	{ uint8_t(0), uint8_t(175), uint8_t(255) },   // 39: #00afff
	{ uint8_t(0), uint8_t(215), uint8_t(0) },     // 40: #00d700
	{ uint8_t(0), uint8_t(215), uint8_t(95) },    // 41: #00d75f
	{ uint8_t(0), uint8_t(215), uint8_t(135) },   // 42: #00d787
	{ uint8_t(0), uint8_t(215), uint8_t(175) },   // 43: #00d7af
	{ uint8_t(0), uint8_t(215), uint8_t(215) },   // 44: #00d7d7
	{ uint8_t(0), uint8_t(215), uint8_t(255) },   // 45: #00d7ff
	{ uint8_t(0), uint8_t(255), uint8_t(0) },     // 46: #00ff00
	{ uint8_t(0), uint8_t(255), uint8_t(95) },    // 47: #00ff5f
	{ uint8_t(0), uint8_t(255), uint8_t(135) },   // 48: #00ff87
	{ uint8_t(0), uint8_t(255), uint8_t(175) },   // 49: #00ffaf
	{ uint8_t(0), uint8_t(255), uint8_t(215) },   // 50: #00ffd7
	{ uint8_t(0), uint8_t(255), uint8_t(255) },   // 51: #00ffff
	{ uint8_t(95), uint8_t(0), uint8_t(0) },      // 52: #5f0000
	{ uint8_t(95), uint8_t(0), uint8_t(95) },     // 53: #5f005f
	{ uint8_t(95), uint8_t(0), uint8_t(135) },    // 54: #5f0087
	{ uint8_t(95), uint8_t(0), uint8_t(175) },    // 55: #5f00af
	{ uint8_t(95), uint8_t(0), uint8_t(215) },    // 56: #5f00d7
	{ uint8_t(95), uint8_t(0), uint8_t(255) },    // 57: #5f00ff
	{ uint8_t(95), uint8_t(95), uint8_t(0) },     // 58: #5f5f00
	{ uint8_t(95), uint8_t(95), uint8_t(95) },    // 59: #5f5f5f
	{ uint8_t(95), uint8_t(95), uint8_t(135) },   // 60: #5f5f87
	{ uint8_t(95), uint8_t(95), uint8_t(175) },   // 61: #5f5faf
	{ uint8_t(95), uint8_t(95), uint8_t(215) },   // 62: #5f5fd7
	{ uint8_t(95), uint8_t(95), uint8_t(255) },   // 63: #5f5fff
	{ uint8_t(95), uint8_t(135), uint8_t(0) },    // 64: #5f8700
	{ uint8_t(95), uint8_t(135), uint8_t(95) },   // 65: #5f875f
	{ uint8_t(95), uint8_t(135), uint8_t(135) },  // 66: #5f8787
	{ uint8_t(95), uint8_t(135), uint8_t(175) },  // 67: #5f87af
	{ uint8_t(95), uint8_t(135), uint8_t(215) },  // 68: #5f87d7
	{ uint8_t(95), uint8_t(135), uint8_t(255) },  // 69: #5f87ff
	{ uint8_t(95), uint8_t(175), uint8_t(0) },    // 70: #5faf00
	{ uint8_t(95), uint8_t(175), uint8_t(95) },   // 71: #5faf5f
	{ uint8_t(95), uint8_t(175), uint8_t(135) },  // 72: #5faf87
	{ uint8_t(95), uint8_t(175), uint8_t(175) },  // 73: #5fafaf
	{ uint8_t(95), uint8_t(175), uint8_t(215) },  // 74: #5fafd7
	{ uint8_t(95), uint8_t(175), uint8_t(255) },  // 75: #5fafff
	{ uint8_t(95), uint8_t(215), uint8_t(0) },    // 76: #5fd700
	{ uint8_t(95), uint8_t(215), uint8_t(95) },   // 77: #5fd75f
	{ uint8_t(95), uint8_t(215), uint8_t(135) },  // 78: #5fd787
	{ uint8_t(95), uint8_t(215), uint8_t(175) },  // 79: #5fd7af
	{ uint8_t(95), uint8_t(215), uint8_t(215) },  // 80: #5fd7d7
	{ uint8_t(95), uint8_t(215), uint8_t(255) },  // 81: #5fd7ff
	{ uint8_t(95), uint8_t(255), uint8_t(0) },    // 82: #5fff00
	{ uint8_t(95), uint8_t(255), uint8_t(95) },   // 83: #5fff5f
	{ uint8_t(95), uint8_t(255), uint8_t(135) },  // 84: #5fff87
	{ uint8_t(95), uint8_t(255), uint8_t(175) },  // 85: #5fffaf
	{ uint8_t(95), uint8_t(255), uint8_t(215) },  // 86: #5fffd7
	{ uint8_t(95), uint8_t(255), uint8_t(255) },  // 87: #5fffff
	{ uint8_t(135), uint8_t(0), uint8_t(0) },     // 88: #870000
	{ uint8_t(135), uint8_t(0), uint8_t(95) },    // 89: #87005f
	{ uint8_t(135), uint8_t(0), uint8_t(135) },   // 90: #870087
	{ uint8_t(135), uint8_t(0), uint8_t(175) },   // 91: #8700af
	{ uint8_t(135), uint8_t(0), uint8_t(215) },   // 92: #8700d7
	{ uint8_t(135), uint8_t(0), uint8_t(255) },   // 93: #8700ff
	{ uint8_t(135), uint8_t(95), uint8_t(0) },    // 94: #875f00
	{ uint8_t(135), uint8_t(95), uint8_t(95) },   // 95: #875f5f
	{ uint8_t(135), uint8_t(95), uint8_t(135) },  // 96: #875f87
	{ uint8_t(135), uint8_t(95), uint8_t(175) },  // 97: #875faf
	{ uint8_t(135), uint8_t(95), uint8_t(215) },  // 98: #875fd7
	{ uint8_t(135), uint8_t(95), uint8_t(255) },  // 99: #875fff
	{ uint8_t(135), uint8_t(135), uint8_t(0) },   // 100: #878700
	{ uint8_t(135), uint8_t(135), uint8_t(95) },  // 101: #87875f
	{ uint8_t(135), uint8_t(135), uint8_t(135) }, // 102: #878787
	{ uint8_t(135), uint8_t(135), uint8_t(175) }, // 103: #8787af
	{ uint8_t(135), uint8_t(135), uint8_t(215) }, // 104: #8787d7
	{ uint8_t(135), uint8_t(135), uint8_t(255) }, // 105: #8787ff
	{ uint8_t(135), uint8_t(175), uint8_t(0) },   // 106: #87af00
	{ uint8_t(135), uint8_t(175), uint8_t(95) },  // 107: #87af5f
	{ uint8_t(135), uint8_t(175), uint8_t(135) }, // 108: #87af87
	{ uint8_t(135), uint8_t(175), uint8_t(175) }, // 109: #87afaf
	{ uint8_t(135), uint8_t(175), uint8_t(215) }, // 110: #87afd7
	{ uint8_t(135), uint8_t(175), uint8_t(255) }, // 111: #87afff
	{ uint8_t(135), uint8_t(215), uint8_t(0) },   // 112: #87d700
	{ uint8_t(135), uint8_t(215), uint8_t(95) },  // 113: #87d75f
	{ uint8_t(135), uint8_t(215), uint8_t(135) }, // 114: #87d787
	{ uint8_t(135), uint8_t(215), uint8_t(175) }, // 115: #87d7af
	{ uint8_t(135), uint8_t(215), uint8_t(215) }, // 116: #87d7d7
	{ uint8_t(135), uint8_t(215), uint8_t(255) }, // 117: #87d7ff
	{ uint8_t(135), uint8_t(255), uint8_t(0) },   // 118: #87ff00
	{ uint8_t(135), uint8_t(255), uint8_t(95) },  // 119: #87ff5f
	{ uint8_t(135), uint8_t(255), uint8_t(135) }, // 120: #87ff87
	{ uint8_t(135), uint8_t(255), uint8_t(175) }, // 121: #87ffaf
	{ uint8_t(135), uint8_t(255), uint8_t(215) }, // 122: #87ffd7
	{ uint8_t(135), uint8_t(255), uint8_t(255) }, // 123: #87ffff
	{ uint8_t(175), uint8_t(0), uint8_t(0) },     // 124: #af0000
	{ uint8_t(175), uint8_t(0), uint8_t(95) },    // 125: #af005f
	{ uint8_t(175), uint8_t(0), uint8_t(135) },   // 126: #af0087
	{ uint8_t(175), uint8_t(0), uint8_t(175) },   // 127: #af00af
	{ uint8_t(175), uint8_t(0), uint8_t(215) },   // 128: #af00d7
	{ uint8_t(175), uint8_t(0), uint8_t(255) },   // 129: #af00ff
	{ uint8_t(175), uint8_t(95), uint8_t(0) },    // 130: #af5f00
	{ uint8_t(175), uint8_t(95), uint8_t(95) },   // 131: #af5f5f
	{ uint8_t(175), uint8_t(95), uint8_t(135) },  // 132: #af5f87
	{ uint8_t(175), uint8_t(95), uint8_t(175) },  // 133: #af5faf
	{ uint8_t(175), uint8_t(95), uint8_t(215) },  // 134: #af5fd7
	{ uint8_t(175), uint8_t(95), uint8_t(255) },  // 135: #af5fff
	{ uint8_t(175), uint8_t(135), uint8_t(0) },   // 136: #af8700
	{ uint8_t(175), uint8_t(135), uint8_t(95) },  // 137: #af875f
	{ uint8_t(175), uint8_t(135), uint8_t(135) }, // 138: #af8787
	{ uint8_t(175), uint8_t(135), uint8_t(175) }, // 139: #af87af
	{ uint8_t(175), uint8_t(135), uint8_t(215) }, // 140: #af87d7
	{ uint8_t(175), uint8_t(135), uint8_t(255) }, // 141: #af87ff
	{ uint8_t(175), uint8_t(175), uint8_t(0) },   // 142: #afaf00
	{ uint8_t(175), uint8_t(175), uint8_t(95) },  // 143: #afaf5f
	{ uint8_t(175), uint8_t(175), uint8_t(135) }, // 144: #afaf87
	{ uint8_t(175), uint8_t(175), uint8_t(175) }, // 145: #afafaf
	{ uint8_t(175), uint8_t(175), uint8_t(215) }, // 146: #afafd7
	{ uint8_t(175), uint8_t(175), uint8_t(255) }, // 147: #afafff
	{ uint8_t(175), uint8_t(215), uint8_t(0) },   // 148: #afd700
	{ uint8_t(175), uint8_t(215), uint8_t(95) },  // 149: #afd75f
	{ uint8_t(175), uint8_t(215), uint8_t(135) }, // 150: #afd787
	{ uint8_t(175), uint8_t(215), uint8_t(175) }, // 151: #afd7af
	{ uint8_t(175), uint8_t(215), uint8_t(215) }, // 152: #afd7d7
	{ uint8_t(175), uint8_t(215), uint8_t(255) }, // 153: #afd7ff
	{ uint8_t(175), uint8_t(255), uint8_t(0) },   // 154: #afff00
	{ uint8_t(175), uint8_t(255), uint8_t(95) },  // 155: #afff5f
	{ uint8_t(175), uint8_t(255), uint8_t(135) }, // 156: #afff87
	{ uint8_t(175), uint8_t(255), uint8_t(175) }, // 157: #afffaf
	{ uint8_t(175), uint8_t(255), uint8_t(215) }, // 158: #afffd7
	{ uint8_t(175), uint8_t(255), uint8_t(255) }, // 159: #afffff
	{ uint8_t(215), uint8_t(0), uint8_t(0) },     // 160: #d70000
	{ uint8_t(215), uint8_t(0), uint8_t(95) },    // 161: #d7005f
	{ uint8_t(215), uint8_t(0), uint8_t(135) },   // 162: #d70087
	{ uint8_t(215), uint8_t(0), uint8_t(175) },   // 163: #d700af
	{ uint8_t(215), uint8_t(0), uint8_t(215) },   // 164: #d700d7
	{ uint8_t(215), uint8_t(0), uint8_t(255) },   // 165: #d700ff
	{ uint8_t(215), uint8_t(95), uint8_t(0) },    // 166: #d75f00
	{ uint8_t(215), uint8_t(95), uint8_t(95) },   // 167: #d75f5f
	{ uint8_t(215), uint8_t(95), uint8_t(135) },  // 168: #d75f87
	{ uint8_t(215), uint8_t(95), uint8_t(175) },  // 169: #d75faf
	{ uint8_t(215), uint8_t(95), uint8_t(215) },  // 170: #d75fd7
	{ uint8_t(215), uint8_t(95), uint8_t(255) },  // 171: #d75fff
	{ uint8_t(215), uint8_t(135), uint8_t(0) },   // 172: #d78700
	{ uint8_t(215), uint8_t(135), uint8_t(95) },  // 173: #d7875f
	{ uint8_t(215), uint8_t(135), uint8_t(135) }, // 174: #d78787
	{ uint8_t(215), uint8_t(135), uint8_t(175) }, // 175: #d787af
	{ uint8_t(215), uint8_t(135), uint8_t(215) }, // 176: #d787d7
	{ uint8_t(215), uint8_t(135), uint8_t(255) }, // 177: #d787ff
	{ uint8_t(215), uint8_t(175), uint8_t(0) },   // 178: #d7af00
	{ uint8_t(215), uint8_t(175), uint8_t(95) },  // 179: #d7af5f
	{ uint8_t(215), uint8_t(175), uint8_t(135) }, // 180: #d7af87
	{ uint8_t(215), uint8_t(175), uint8_t(175) }, // 181: #d7afaf
	{ uint8_t(215), uint8_t(175), uint8_t(215) }, // 182: #d7afd7
	{ uint8_t(215), uint8_t(175), uint8_t(255) }, // 183: #d7afff
	{ uint8_t(215), uint8_t(215), uint8_t(0) },   // 184: #d7d700
	{ uint8_t(215), uint8_t(215), uint8_t(95) },  // 185: #d7d75f
	{ uint8_t(215), uint8_t(215), uint8_t(135) }, // 186: #d7d787
	{ uint8_t(215), uint8_t(215), uint8_t(175) }, // 187: #d7d7af
	{ uint8_t(215), uint8_t(215), uint8_t(215) }, // 188: #d7d7d7
	{ uint8_t(215), uint8_t(215), uint8_t(255) }, // 189: #d7d7ff
	{ uint8_t(215), uint8_t(255), uint8_t(0) },   // 190: #d7ff00
	{ uint8_t(215), uint8_t(255), uint8_t(95) },  // 191: #d7ff5f
	{ uint8_t(215), uint8_t(255), uint8_t(135) }, // 192: #d7ff87
	{ uint8_t(215), uint8_t(255), uint8_t(175) }, // 193: #d7ffaf
	{ uint8_t(215), uint8_t(255), uint8_t(215) }, // 194: #d7ffd7
	{ uint8_t(215), uint8_t(255), uint8_t(255) }, // 195: #d7ffff
	{ uint8_t(255), uint8_t(0), uint8_t(0) },     // 196: #ff0000
	{ uint8_t(255), uint8_t(0), uint8_t(95) },    // 197: #ff005f
	{ uint8_t(255), uint8_t(0), uint8_t(135) },   // 198: #ff0087
	{ uint8_t(255), uint8_t(0), uint8_t(175) },   // 199: #ff00af
	{ uint8_t(255), uint8_t(0), uint8_t(215) },   // 200: #ff00d7
	{ uint8_t(255), uint8_t(0), uint8_t(255) },   // 201: #ff00ff
	{ uint8_t(255), uint8_t(95), uint8_t(0) },    // 202: #ff5f00
	{ uint8_t(255), uint8_t(95), uint8_t(95) },   // 203: #ff5f5f
	{ uint8_t(255), uint8_t(95), uint8_t(135) },  // 204: #ff5f87
	{ uint8_t(255), uint8_t(95), uint8_t(175) },  // 205: #ff5faf
	{ uint8_t(255), uint8_t(95), uint8_t(215) },  // 206: #ff5fd7
	{ uint8_t(255), uint8_t(95), uint8_t(255) },  // 207: #ff5fff
	{ uint8_t(255), uint8_t(135), uint8_t(0) },   // 208: #ff8700
	{ uint8_t(255), uint8_t(135), uint8_t(95) },  // 209: #ff875f
	{ uint8_t(255), uint8_t(135), uint8_t(135) }, // 210: #ff8787
	{ uint8_t(255), uint8_t(135), uint8_t(175) }, // 211: #ff87af
	{ uint8_t(255), uint8_t(135), uint8_t(215) }, // 212: #ff87d7
	{ uint8_t(255), uint8_t(135), uint8_t(255) }, // 213: #ff87ff
	{ uint8_t(255), uint8_t(175), uint8_t(0) },   // 214: #ffaf00
	{ uint8_t(255), uint8_t(175), uint8_t(95) },  // 215: #ffaf5f
	{ uint8_t(255), uint8_t(175), uint8_t(135) }, // 216: #ffaf87
	{ uint8_t(255), uint8_t(175), uint8_t(175) }, // 217: #ffafaf
	{ uint8_t(255), uint8_t(175), uint8_t(215) }, // 218: #ffafd7
	{ uint8_t(255), uint8_t(175), uint8_t(255) }, // 219: #ffafff
	{ uint8_t(255), uint8_t(215), uint8_t(0) },   // 220: #ffd700
	{ uint8_t(255), uint8_t(215), uint8_t(95) },  // 221: #ffd75f
	{ uint8_t(255), uint8_t(215), uint8_t(135) }, // 222: #ffd787
	{ uint8_t(255), uint8_t(215), uint8_t(175) }, // 223: #ffd7af
	{ uint8_t(255), uint8_t(215), uint8_t(215) }, // 224: #ffd7d7
	{ uint8_t(255), uint8_t(215), uint8_t(255) }, // 225: #ffd7ff
	{ uint8_t(255), uint8_t(255), uint8_t(0) },   // 226: #ffff00
	{ uint8_t(255), uint8_t(255), uint8_t(95) },  // 227: #ffff5f
	{ uint8_t(255), uint8_t(255), uint8_t(135) }, // 228: #ffff87
	{ uint8_t(255), uint8_t(255), uint8_t(175) }, // 229: #ffffaf
	{ uint8_t(255), uint8_t(255), uint8_t(215) }, // 230: #ffffd7
	{ uint8_t(255), uint8_t(255), uint8_t(255) }, // 231: #ffffff

	// gray scale
	{ uint8_t(8), uint8_t(8), uint8_t(8) },       // 232: #080808
	{ uint8_t(18), uint8_t(18), uint8_t(18) },    // 233: #121212
	{ uint8_t(28), uint8_t(28), uint8_t(28) },    // 234: #1c1c1c
	{ uint8_t(38), uint8_t(38), uint8_t(38) },    // 235: #262626
	{ uint8_t(48), uint8_t(48), uint8_t(48) },    // 236: #303030
	{ uint8_t(58), uint8_t(58), uint8_t(58) },    // 237: #3a3a3a
	{ uint8_t(68), uint8_t(68), uint8_t(68) },    // 238: #444444
	{ uint8_t(78), uint8_t(78), uint8_t(78) },    // 239: #4e4e4e
	{ uint8_t(88), uint8_t(88), uint8_t(88) },    // 240: #585858
	{ uint8_t(98), uint8_t(98), uint8_t(98) },    // 241: #626262
	{ uint8_t(108), uint8_t(108), uint8_t(108) }, // 242: #6c6c6c
	{ uint8_t(118), uint8_t(118), uint8_t(118) }, // 243: #767676
	{ uint8_t(128), uint8_t(128), uint8_t(128) }, // 244: #808080
	{ uint8_t(138), uint8_t(138), uint8_t(138) }, // 245: #8a8a8a
	{ uint8_t(148), uint8_t(148), uint8_t(148) }, // 246: #949494
	{ uint8_t(158), uint8_t(158), uint8_t(158) }, // 247: #9e9e9e
	{ uint8_t(168), uint8_t(168), uint8_t(168) }, // 248: #a8a8a8
	{ uint8_t(178), uint8_t(178), uint8_t(178) }, // 249: #b2b2b2
	{ uint8_t(188), uint8_t(188), uint8_t(188) }, // 250: #bcbcbc
	{ uint8_t(198), uint8_t(198), uint8_t(198) }, // 251: #c6c6c6
	{ uint8_t(208), uint8_t(208), uint8_t(208) }, // 252: #d0d0d0
	{ uint8_t(218), uint8_t(218), uint8_t(218) }, // 253: #dadada
	{ uint8_t(228), uint8_t(228), uint8_t(228) }, // 254: #e4e4e4
	{ uint8_t(238), uint8_t(238), uint8_t(238) }  // 255: #eeeeee
};
