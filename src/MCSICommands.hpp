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

// List with known CSI commands

#pragma once

//	eDECBI =		'****', 	// Back index
//	eDECFI =		'****', 	// Forward index
//	eLS1R =			'****', 	// Locking shift 1 right
//	eLS2 =			'****', 	// Locking shift 2
//	eLS2R =			'****', 	// Locking shift 2 right
//	eLS3 =			'****', 	// Locking shift 3
//	eLS3R =			'****', 	// Locking shift 3 right
//	eS7C1T =		'****', 	// Send 7-bit C1 controls
//	eS8C1T =		'****', 	// Send 8-bit C1 controls

//	eDECDLD =		'****', 	// Downline-loadable set
//	eDECDMAC =		'****', 	// Define macro

//	eDECUDK =		'****', 	// User-defined keys

//	eDECPSR =		'****', 	// Presentation state report
//	eDECTABSR =		'****', 	// Tabulation stop report
//	eDECCIR =		'****', 	// Cursor information report

//	eDECCKSR =		'****', 	// Checksum report

//	eDECRPDE =		'****', 	// Report displayed extent
//	eDECRPSS =		'****', 	// Report selection or setting
//	eDECRPTUI =		'****', 	// Report terminal unit ID
//	eDECRQSS =		'****', 	// Request selection or setting

//	eDECRSPS =		'****', 	// Restore presentation state
//	eDECRSTS =		'****', 	// Restore terminal state

//	eDECAUPSS =		'****', 	// Assign user-preferred supplemental set

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmultichar"

enum MCSICmd : uint32_t
{
	eCBT = 'Z',       // Cursor Backward Tabulation
	eCHA = 'G',       // Cursor Horizontal Absolute
	eCHT = 'I',       // Cursor Horizontal Forward Tab
	eCNL = 'E',       // Cursor Next Line
	eCPL = 'F',       // Cursor Previous Line
	eCUB = 'D',       // Cursor Backward
	eCUD = 'B',       // Cursor Down
	eCUF = 'C',       // Cursor Forward
	eCUP = 'H',       // Cursor Position
	eCUU = 'A',       // Cursor Up
	eDA1 = 'c',       // Response to Device Attributes
	eDA2 = '>c',      // Secondary Device Attributes
	eDA3 = '=c',      // Tertiary Device Attributes
	eDCH = 'P',       // Delete Character
	eDECCOLM = '$|',  // Set columns per page
	eDL = 'M',        // Delete Line(s)
	eDSR1 = 'n',      // Device Status Report ANSI
	eDSR2 = '?n',     // Device Status Report DEC
	eED = 'J',        // Erase in Display
	eEL = 'K',        // Erase in Line
	eHPA = '`',       // Horizontal Position Absolute
	eHPR = 'a',       // Horizontal Position Relative
	eHVP = 'f',       // Horizontal/Vertical Position
	eIL = 'L',        // Insert Line
	eMC = 'i',        // Start Printer-to-Host Session
	eNP = 'U',        // Next Page
	ePP = 'V',        // Preceding Page
	ePPA = ' P',      // Page Position Absolute
	ePPB = ' R',      // Page Position Backwards
	ePPR = ' Q',      // Page Position Relative
	eREP = 'b',       // Repeat preceding character
	eSAVEMODE = '?s', // Save DEC Private Mode Values
	eRM_ANSI = 'l',   // Reset Mode ANSI
	eRM_DEC = '?l',   // Reset Mode DEC
	eRESTMODE = '?r', // Restore DEC Private Mode Values
	eSCORC = 'u',     // Restore Saved Cursor Position (SCO Console)
					  //	eSCOSC =		's',		// Save Current Cursor Position (SCO Console)
	eDECSLRM = 's',   // Set left and right margins
	eSD = 'T',        // Pan Up
	eSGR = 'm',       // Select Graphic Rendition
	eSL = ' @',       // Shift Left
	eSM_ANSI = 'h',   // Set Mode ANSI
	eSM_DEC = '?h',   // Set Mode DEC
	eSR = ' A',       // Shift Right
	eSU = 'S',        // Pan Down
	eTBC = 'g',       // Clear Tabs
	eVPA = 'd',       // Vertical Line Position Absolute
	eVPR = 'e',       // Vertical Position Relative

	eXTERMDMK = '>n', // Set XTerm modify keys
	eXTERMEMK = '>m', // Reset XTerm modify keys
	eXTERMDMKR = '?m',// Get XTerm modify keys state

	eDECCARA = '$r',    // Change attributes in rectangular area
	eDECCRA = '$v',     // Copy rectangular area
	eDECDC = '\'~',     // Delete column
	eDECELF = '+q',     // Enable local functions
	eDECERA = '$z',     // Erase rectangular area
	eDECFRA = '$x',     // Fill rectangular area
	eDECIC = '\'}',     // Insert column
	eDECINVM = '*z',    // Invoke stored macro
	eDECLFKC = '*g',    // Local function key control
	eDECMSR = '*{',     // Macro Space Report
	eDECRARA = '$t',    // Reverse attributes in rectangular area
	eDECRPM = '$y',     // Report mode
	eDECRQCRA = '*y',   // Request checksum of rectangular area
	eDECRQDE = '"v',    // Request displayed extent
	eDECRQMANSI = '$p', // Request mode ANSI
	eDECRQMDEC = '?$p', // Request mode DEC Private
	eDECRQPSR = '$w',   // Request presentation state
	eDECRQTSR = '$u',   // Request Terminal state
	eDECRQUPSS = '&u',  // Request User-Preferred Supplemental Set
	eDECSACE = '*x',    // Select attribute change extent
	eDECSASD = '$}',    // Select active status display
	eDECSCA = '"q',     // Select character attribute
	eDECSCL = '"p',     // Select Conformance Level
	eDECSED = '?J',     // Selective erase in display
	eDECSEL = '?K',     // Selective erase in line
	eDECSERA = '${',    // Selective erase rectangular area
	eDECSLPP = 't',     // Set Lines Per Page
	eDECSMKR = '+r',    // Select modifier key reporting
	eDECSMBV = ' u',    // Set Margin Bell Volume
	eDECSNLS = '*|',    // Select number of lines per screen
	eDECSR = '+p',      // Secure reset
	eDECSSDT = '$~',    // Select status display type
	eDECSTBM = 'r',     // Set Top and Bottom Margin
	eDECSTR = '!p',     // Soft terminal reset
	eDECSWBV = ' t',    // Set Warning Bell Volume
	eECH = 'X',         // Erase character
	eICH = '@',         // Insert character

	//	eDECSF =		'****', 	// Select font
	//	eDECSTUI =		'****', 	// Set terminal unit ID

	/*
	        DEC VT520 requests
	*/

	eDECSCUSR = ' q', // Set Cursor Style

	/*
	        XTerm requests
	*/

	eDECREQTPARM = 'x', // no comment

	eDECELR = '\'z', // mouse events

	//	eDECAC =		',|',		// Assign Color
	//	eDECARR =		'-p',		// Auto Repeat Rate
	//	eDECATC =		',g',		// Alternate Text Colors
	//	eDECCRTST =		'-q',		// Saver Timing
	//	eDECDLDA =		',z',		// Down Line Load Allocation
	//	eDECES =		'&x',		// Enable Session
	//	eDECFNK =		'~',		// Function Key
	//	eDECKBD =		' g',		// Keyboard Language Selection
	//	eDECLL =		'q',		// Load LEDs
	//	eDECLTOD =		',p',		// Load Time of Day
	//	eDECPCTERM =	'?r',		// Enter/Exit PCTerm Mode
	//	eDECPKA =		'+z',		// Program Key Action
	//	eDECPKFMR =		'+y',		// Program Key Free Memory Report
	//	eDECPS =		',~',		// Play Sound
	//	eDECRPKT =		',v',		// Report Key Type
	//	eDECRQKD =		',w',		// Key Definition Inquiry
	//	eDECRQKT =		',u',		// Key Type Inquiry
	//	eDECRQPKFM =	'+x',		// Key Free Memory Inquiry
	//	eDECSCP =		'*u',		// Select Communication Port
	//	eDECSCS =		'*r',		// Select Communication Speed
	//	eDECSDDT =		'$q',		// Select Disconnect Delay Time
	//	eDECSDPT =		')p',		// Select Digital Printed Data Type
	//	eDECSEST =		'-r',		// Energy Saver Timing
	//	eDECSFC =		'*s',		// Select Flow Control
	//	eDECSKCV =		' r',		// Set Key Click Volume
	//	eDECSLCK =		' v',		// Set Lock Key Style
	//	eDECSPMA =		',x',		// Session Page Memory Allocation
	//	eDECSPP =		'+w',		// Set Port Parameter
	//	eDECSPPCS =		'*p',		// Select ProPrinter Character Set
	//	eDECSPRTT =		'$s',		// Select Printer Type
	//	eDECSSCLS =		' p',		// Set Scroll Speed
	//	eDECSSL =		'p',		// Select Set-Up Language
	//	eDECST8C =		'?W',		// Set Tab at every 8 columns
	//	eDECSTGLT =		'){',		// Select Color Look-Up Table
	//	eDECSTRL =		'"u',		// Set Transmit Rate Limit
	//	eDECSZS =		',{',		// Select Zero Symbol
	//	eDECTID =		',q',		// Select Terminal ID
	//	eDECTME =		' ~',		// Terminal Mode Emulation
	//	eDECTST =		'y',		// Invoke Confidence Test
	//	eDECUS =		',y',		// Update Session
	
};

//	eDECKBUM =		'****', 	// Keyboard usage mode
//	eDECKPM =		'****', 	// Key position mode
//	eDECNKM =		'****', 	// Numeric keypad mode
//	eDECVSSM =		'****', 	// Vertical split-screen mode

#pragma GCC diagnostic pop

#if DEBUG

struct MCmdName
{
	uint32_t cmd;
	const char *name;
} kCmdNames[] = {
	{ eCBT, "CBT" },
	{ eCHA, "CHA" },
	{ eCHT, "CHT" },
	{ eCNL, "CNL" },
	{ eCPL, "CPL" },
	{ eCUB, "CUB" },
	{ eCUD, "CUD" },
	{ eCUF, "CUF" },
	{ eCUP, "CUP" },
	{ eCUU, "CUU" },
	{ eDA1, "DA1" },
	{ eDA2, "DA2" },
	{ eDA3, "DA3" },
	{ eDCH, "DCH" },
	{ eDECCOLM, "DECCOLM" },
	{ eDL, "DL" },
	{ eDSR1, "DSR1" },
	{ eDSR2, "DSR2" },
	{ eED, "ED" },
	{ eEL, "EL" },
	{ eHPA, "HPA" },
	{ eHPR, "HPR" },
	{ eHVP, "HVP" },
	{ eIL, "IL" },
	{ eMC, "MC" },
	{ eNP, "NP" },
	{ ePP, "PP" },
	{ ePPA, "PPA" },
	{ ePPB, "PPB" },
	{ ePPR, "PPR" },
	{ eREP, "REP" },
	{ eSAVEMODE, "SAVEMODE" },
	{ eRM_ANSI, "RM_ANSI" },
	{ eRM_DEC, "RM_DEC" },
	{ eRESTMODE, "RESTMODE" },
	{ eSCORC, "SCORC" },
	{ eDECSLRM, "DECSLRM" },
	{ eSD, "SD" },
	{ eSGR, "SGR" },
	{ eSL, "SL" },
	{ eSM_ANSI, "SM_ANSI" },
	{ eSM_DEC, "SM_DEC" },
	{ eSR, "SR" },
	{ eSU, "SU" },
	{ eTBC, "TBC" },
	{ eVPA, "VPA" },
	{ eVPR, "VPR" },
	{ eXTERMDMK, "XTERMDMK" },
	{ eXTERMEMK, "XTERMEMK" },
	{ eDECCARA, "DECCARA" },
	{ eDECCRA, "DECCRA" },
	{ eDECDC, "DECDC" },
	{ eDECELF, "DECELF" },
	{ eDECERA, "DECERA" },
	{ eDECFRA, "DECFRA" },
	{ eDECIC, "DECIC" },
	{ eDECINVM, "DECINVM" },
	{ eDECLFKC, "DECLFKC" },
	{ eDECMSR, "DECMSR" },
	{ eDECRARA, "DECRARA" },
	{ eDECRPM, "DECRPM" },
	{ eDECRQCRA, "DECRQCRA" },
	{ eDECRQDE, "DECRQDE" },
	{ eDECRQMANSI, "DECRQMANSI" },
	{ eDECRQMDEC, "DECRQMDEC" },
	{ eDECRQPSR, "DECRQPSR" },
	{ eDECRQTSR, "DECRQTSR" },
	{ eDECRQUPSS, "DECRQUPSS" },
	{ eDECSACE, "DECSACE" },
	{ eDECSASD, "DECSASD" },
	{ eDECSCA, "DECSCA" },
	{ eDECSCL, "DECSCL" },
	{ eDECSED, "DECSED" },
	{ eDECSEL, "DECSEL" },
	{ eDECSERA, "DECSERA" },
	{ eDECSLPP, "DECSLPP" },
	{ eDECSMKR, "DECSMKR" },
	{ eDECSMBV, "DECSMBV" },
	{ eDECSNLS, "DECSNLS" },
	{ eDECSR, "DECSR" },
	{ eDECSSDT, "DECSSDT" },
	{ eDECSTBM, "DECSTBM" },
	{ eDECSTR, "DECSTR" },
	{ eDECSWBV, "DECSWBV" },
	{ eECH, "ECH" },
	{ eICH, "ICH" },
	{ eDECSCUSR, "DECSCUSR" },
	{ eDECREQTPARM, "DECREQTPARM" },
	{ 0, "" }
};

const char *Cmd2Name(uint32_t cmd)
{
	const char *result = "(unknown)";

	for (auto e : kCmdNames)
	{
		if (e.cmd == cmd)
		{
			result = e.name;
			break;
		}
	}

	return result;
}

#endif
