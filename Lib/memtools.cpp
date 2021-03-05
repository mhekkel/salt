/*
	Memtools.c
	Copyright (c) Bas Vodde.

	Memory tools are small C/C++ memory tools which can be used for debugging memory leaks
	or profiling memory consumption (on the heap). Its based on hooks in the allocate loop!
	This means that it is not ment to "take over" the memory management but everytime a alloc
	call is made a hook will do "some" work for administration or other purposes.

	Everything is generic. There can be printing hooks installed for printing to different
	outputs than stderr. This is especially usefull for environments who support output windows.
*/

#include "MLib.h"

#include "memtools.h"
#include "MError.h"

#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "assert.h"

/* Node which stores the memory details */
struct memInfoNode
{
	const char*				fFileName;
	long					fLineNum;
	int						fIsArray;
	int						fIsAllowed;
	size_t					fSize;
	unsigned long			fID;
	void*					fMemory;
	struct memInfoNode*		fNext;
};

/* List which stores the memory nodes */
struct memInfoList
{
	struct memInfoNode*		fHead;
};

struct memInfoNode*		memListFind (struct memInfoList* inList, void* inMemory);
int						memListRemove (struct memInfoList* inList, void* inMemory);
int 					memListRemoveNoDelete (struct memInfoList* inList, void* inMemory, struct memInfoNode** outNode);
void					memListAdd (struct memInfoList* inList, void* inMemory, const char* inFilename, long inLineNum, size_t inSize, int inArray);
void					memListDelete (struct memInfoList* inList);
void					memListClear (struct memInfoList* inList);

/* Hashing table for hashing the allocated memory. */
enum {hash_Prime = 9973};
struct memInfoTable
{
	struct memInfoList		fTable[hash_Prime];
};

struct memInfoNode*		memTableFind (struct memInfoTable* inTable, void* inMemory);
int 					memTableRemove (struct memInfoTable* inTable, void* inMemory);
int 					memTableRemoveNoDelete (struct memInfoTable* inTable, void* inMemory, struct memInfoNode** outNode);
void					memTableAdd (struct memInfoTable* inTable, void* inMemory, const char* inFilename, long inLineNum, size_t inSize, int inArray);
void					memTableDeleteAll (struct memInfoTable* inTable);
void					memTableClear (struct memInfoTable* inTable);
unsigned long			memTableHash (void* inPtr);


struct memInfoTable		memMemoryTable;
unsigned long			memCurID = 0;

void					memSortMemLeaks (struct memInfoNode* inLeak, int inSize);
void					memSortMemLeaks2 (struct memInfoNode* inLeak, int inLeft, int inRight);
int						memSplitMemLeakArray (struct memInfoNode* inLeak, int inLeft, int inRight);

void					memSwapMemLeak (struct memInfoNode* inNode1, struct memInfoNode* inNode2);
void					memConstructMemReport (char* outBuffer, int inBufSize, const char* inFileName, int inLineNum, int inSize, void* inMem, const char inErrString[]);

void (*memPrintHook) (const char*) = NULL;
void (*memAssertHook) () = NULL;

void					memPrintError (const char* inText);
void					memPrintDamage (struct memInfoNode* inNode);
void					memPrintDeleteArray (struct memInfoNode* inNode);
void					memPrintLeak (struct memInfoNode* inLeak);
void					memAssert ();

// Hooks
void					memLeakDetectorNew (void* inMemory, size_t inSize, const char* inFilename, long inLineNum);
void					memLeakDetectorDelete (void* inMemory);

/* Memory list implementations */

__inline
struct memInfoNode*		
memListFind (struct memInfoList* inList, void* inMemory)
{
	struct memInfoNode* lPtr = inList->fHead;
	while (lPtr) {
		if (lPtr->fMemory == inMemory)
			return lPtr;
		lPtr = lPtr->fNext;
	}
	return NULL;
}

__inline
void					
memListAdd (struct memInfoList* inList, void* inMemory, const char* inFilename, long inLineNum, size_t inSize, int inArray)
{
	struct memInfoNode* lNewNode = (struct memInfoNode*) malloc (sizeof (struct memInfoNode));
	if (lNewNode) {
		lNewNode->fMemory = inMemory;
		lNewNode->fFileName = inFilename;
		lNewNode->fLineNum = inLineNum;
		lNewNode->fSize = inSize;
		lNewNode->fID = memCurID++;
		lNewNode->fIsArray = inArray;
		lNewNode->fIsAllowed = 0;
		lNewNode->fNext = inList->fHead;
		inList->fHead = lNewNode;
	}
}

__inline
int
memListRemoveNoDelete (struct memInfoList* inList, void* inMemory, struct memInfoNode** outNode)
{
	struct memInfoNode* lPtr = inList->fHead;
	struct memInfoNode* lPrev = NULL;
	while (lPtr) {
		if (lPtr->fMemory == inMemory) {
			if (lPrev) {
				lPrev->fNext = lPtr->fNext;
				*outNode = lPtr;
				return 1;
			}
			else {
				inList->fHead = lPtr->fNext;
				*outNode = lPtr;
				return 1;
			}

		}
		lPrev = lPtr;
		lPtr = lPtr->fNext;
	}
	return 0;
}


__inline
int
memListRemove (struct memInfoList* inList, void* inMemory)
{
	struct memInfoNode* lNode = NULL;
	int lResult = memListRemoveNoDelete (inList, inMemory, &lNode);
	free (lNode);
	return lResult;
}

void					
memListDeleteAll (struct memInfoList* inList)
{
	struct memInfoNode* lPtr = inList->fHead;
	struct memInfoNode* lNext = NULL;
	while (lPtr) {
		lNext = lPtr->fNext;
		free (lPtr->fMemory);
		lPtr = lNext;
	}
}

void					
memListClear (struct memInfoList* inList)
{
	struct memInfoNode* lPtr = inList->fHead;
	struct memInfoNode* lNext = NULL;
	while (lPtr) {
		lNext = lPtr->fNext;
		free (lPtr);
		lPtr = lNext;
	}
	inList->fHead = NULL;
}

/* Memory table implementation */
struct memInfoNode*		
memTableFind (struct memInfoTable* inTable, void* inMemory)
{
	unsigned lHash = memTableHash (inMemory);
	return memListFind (&inTable->fTable[lHash], inMemory);
}

int
memTableRemove (struct memInfoTable* inTable, void* inMemory)
{
	unsigned lHash = memTableHash (inMemory);
	return memListRemove (&inTable->fTable[lHash], inMemory);
}

int
memTableRemoveNoDelete (struct memInfoTable* inTable, void* inMemory, struct memInfoNode** outNode)
{
	unsigned lHash = memTableHash (inMemory);
	return memListRemoveNoDelete (&inTable->fTable[lHash], inMemory, outNode);
}


void					
memTableAdd (struct memInfoTable* inTable, void* inMemory, const char* inFilename, long inLineNum, size_t inSize, int inArray)
{
	unsigned lHash = memTableHash (inMemory);
	memListAdd (&inTable->fTable[lHash], inMemory, inFilename, inLineNum, inSize, inArray);
}


void					
memTableDeleteAll (struct memInfoTable* inTable)
{
	int i;
	for (i = 0; i < hash_Prime; ++i) {
		memListDeleteAll (&inTable->fTable[i]);
	}
}	

void					
memTableClear (struct memInfoTable* inTable)
{
	int i;
	for (i = 0; i < hash_Prime; ++i) {
		memListClear (&inTable->fTable[i]);
	}
}	

unsigned long
memTableHash (void* inPtr)
{
	return ((unsigned long) (inPtr) % hash_Prime);
}

void 
memSwapMemLeak (struct memInfoNode* inNode1, struct memInfoNode* inNode2)
{
	struct memInfoNode lTmp = *inNode1;
	*inNode1 = *inNode2;
	*inNode2 = lTmp;
}

/* Quicksort the memory leaks.*/
void
memSortMemLeaks (struct memInfoNode* inLeak, int inSize)
{
	memSortMemLeaks2 (inLeak, 0, inSize-1);
}

void 
memSortMemLeaks2 (struct memInfoNode* inLeak, int inLeft, int inRight)
{
	if (inRight > inLeft) {
		int lIndex = memSplitMemLeakArray (inLeak, inLeft, inRight);
		memSortMemLeaks2 (inLeak, inLeft, lIndex-1);
		memSortMemLeaks2 (inLeak, lIndex+1, inRight);
	}
   
}

int 
memSplitMemLeakArray (struct memInfoNode* inLeak, int inLeft, int inRight)
{
	unsigned lPivot = inLeak[inRight].fID;
	int i = inLeft -1;
	int j = inRight;
	for (;;) {
		while (inLeak[++i].fID < lPivot) {
		}
		while (--j && inLeak[j].fID > lPivot) {
		}
		if (i >= j) {
			break;
		}
		memSwapMemLeak (&inLeak[i], &inLeak[j]);
	}
	memSwapMemLeak (&inLeak[i], &inLeak[inRight]);
	return i;
}

void					
memInstallPrintHook (void (*inPrintHook) (const char*))
{
	memPrintHook = inPrintHook;
}

void					
memInstallAssertHook (void (*inAssertHook) ())
{
	memAssertHook = inAssertHook;
}

void 
memPrintError (const char* inText) 
{
	if (memPrintHook) {
		(*memPrintHook) (inText);
	}
	else {
		__debug_printf(inText);
	}
}

void
memAssert ()
{
	if (memAssertHook) {
		(*memAssertHook) ();
	}
	else {
		assert (0);
	}
}


void 
memPrintDamage (struct memInfoNode* inNode)
{
	static const char lFormatText[] = "Damaged memory (out of bounds write?).";
	enum {sPrintBufferSize = 1024};
	static char sPrintBuf [sPrintBufferSize];
	memConstructMemReport (sPrintBuf, sPrintBufferSize, inNode->fFileName, inNode->fLineNum, inNode->fSize, inNode->fMemory, lFormatText);
	memPrintError (sPrintBuf);
}

void 
memPrintDeleteArray (struct memInfoNode* inNode)
{
	static const char lFormatText[] = "Deleting an array (or non-array) while differently allocated";
	enum {sPrintBufferSize = 1024};
	static char sPrintBuf [sPrintBufferSize];
	memConstructMemReport (sPrintBuf, sPrintBufferSize, inNode->fFileName, inNode->fLineNum, inNode->fSize, inNode->fMemory, lFormatText);
	memPrintError (sPrintBuf);
}


void
memMemoryToDebugHexString (char* outBuffer, char* inMemory, int inMemorySize)
{
	static char lFormatText[] = "Data: ";
	int i = 0;
	char lChar1;
	char lChar2;

	/* Add the info text */
	memcpy (outBuffer, lFormatText, sizeof (lFormatText) -1);
	outBuffer += sizeof (lFormatText) -1;

	/* Do the hex output */
	for (i = 0 ; i < inMemorySize ; ++i) {
		lChar1 = (*inMemory >> 4) & 0x0F;
		lChar2 = *inMemory & 0x0F;

		*outBuffer++ = lChar1 + ((lChar1 < 10) ? '0' : ('A' - 10));
		*outBuffer++ = lChar2 + ((lChar2 < 10) ? '0' : ('A' - 10));
		*outBuffer++ = ' ';
		++inMemory;
	}
	*outBuffer++ = '\n';
	*outBuffer++ = '\0';
}

void
memMemoryToDebugTextString (char* outBuffer, char* inMemory, int inMemorySize)
{
	static char lFormatText[] = "Text: ";
	int i;

	/* Add the info text */
	memcpy (outBuffer, lFormatText, sizeof (lFormatText) -1);
	outBuffer += sizeof (lFormatText) -1;

	/* Do the text output */
	memcpy (outBuffer, inMemory, inMemorySize);

	for (i = 0; i < inMemorySize; ++i) {
		if (*outBuffer == '\0') {
			break;
		}
		++outBuffer;
	}
	*outBuffer++ = '\n';
	*outBuffer++ = '\0';
}

void
memPrintLeak (struct memInfoNode* inLeak)
{
	static const char lFormatText[] = "Memory leak.";
	
	enum {sPrintBufferSize = 1024};
	static char sPrintBuf [sPrintBufferSize];
	unsigned int lSize = (inLeak->fSize > 80) ? 80 : inLeak->fSize;

	/* Construct the memory leak error message */
	memConstructMemReport (sPrintBuf, sPrintBufferSize, inLeak->fFileName, inLeak->fLineNum, inLeak->fSize, inLeak->fMemory, lFormatText);
	memPrintError (sPrintBuf);

	/* Print the memory. Maximum of 80 characters. */
	memMemoryToDebugHexString (sPrintBuf, (char*)inLeak->fMemory, lSize);
	memPrintError (sPrintBuf);

	/* Print in text */	
	memMemoryToDebugTextString (sPrintBuf, (char*)inLeak->fMemory, lSize);
	memPrintError (sPrintBuf);
}


void
memPtrToString (char* outBuffer, char* inMemory)
{
	char* lMem = (char*)&inMemory;
	int i = 0;
	outBuffer += 7;

	for (i = 0 ; i < 4 ; ++i) {
		char lChar1 = *lMem & 0x0F;
		char lChar2 = (*lMem >> 4) & 0x0F;
		*outBuffer-- = lChar1 + ((lChar1 < 10) ? '0' : ('A' - 10));
		*outBuffer-- = lChar2 + ((lChar2 < 10) ? '0' : ('A' - 10));
		++lMem;
	}
	outBuffer[9] = '\0';
}


void
memConstructMemReport (char* outBuffer, int inBufSize, const char* inFileName, int inLineNum, int inSize, void* inMem, const char inErrString[])
{
	/*
		No bounds checking for the outBuffer. The buffer should be at least 2 times MAXPATH 
	*/
	static const char lFormatText1[] = "): ";
	static const char lFormatText2[] = " Size: ";
	static const char lFormatText3[] = " Address: 0x";
	static char lNumBuffer[20];

	char* lPtr = outBuffer;
	
	/* Copy the filename */
	int lSize = strlen (inFileName);
	memcpy (lPtr, inFileName, lSize);
	lPtr += lSize;

	/* Some formatting */
	*lPtr++ = '(';

	/* Fix the line number */
	itoa (inLineNum, lNumBuffer, 10);
	lSize = strlen (lNumBuffer);
	memcpy (lPtr, lNumBuffer, lSize);
	lPtr += lSize;

	/* Format 1 */
	lSize = sizeof (lFormatText1) - 1;
	memcpy (lPtr, lFormatText1, lSize);
	lPtr += lSize;

	/* Error string */
	lSize = strlen (inErrString);
	memcpy (lPtr, inErrString, lSize);
	lPtr += lSize;

	/* Format 2 */
	lSize = sizeof (lFormatText2) - 1;
	memcpy (lPtr, lFormatText2, lSize);
	lPtr += lSize;

	/* Size */
	itoa (inSize, lNumBuffer, 10);
	lSize = strlen (lNumBuffer);
	memcpy (lPtr, lNumBuffer, lSize);
	lPtr += lSize;

	/* Format 3 */
	lSize = sizeof (lFormatText3) - 1;
	memcpy (lPtr, lFormatText3, lSize);
	lPtr += lSize;

	/* Memory address */
	memPtrToString (lNumBuffer, (char*)inMem);
	memcpy (lPtr, lNumBuffer, 8);
	lPtr += 8;

	/* Ending */
	*lPtr++ = '\n';
	*lPtr++ = '\0';
}

/* Public interface functions */

void
memLeakDetectorAlloc (void* inMemory, size_t inSize, const char* inFilename, long inLineNum, int inArray)
{
	memTableAdd (&memMemoryTable, inMemory, inFilename, inLineNum, inSize, inArray);
	((char*) inMemory)[inSize] = 'B';
	((char*) inMemory)[inSize+1] = 'A';
	((char*) inMemory)[inSize+2] = 'S';
}

void
memLeakDetectorFree (void* inMemory, int inArray)
{
	char* lMemCheck;
	struct memInfoNode* lNode = NULL;
	if (inMemory == NULL) {
		return;
	}
	if (!memTableRemoveNoDelete (&memMemoryTable, inMemory, &lNode)) {
		memPrintError ("Possibly deleting non allocated memory. Memory was not allocated when the memory leak detector was enabled.\n");
		memAssert ();
		return;
	}
	lMemCheck = (char*) lNode->fMemory;
	if (lMemCheck[lNode->fSize] != 'B' ||
		lMemCheck[lNode->fSize+1] != 'A' || 
		lMemCheck[lNode->fSize+2] != 'S') {
		memPrintDamage (lNode);
		memAssert ();
	}
	else if ((inArray != 0) != (lNode->fIsArray != 0)) {
		memPrintDeleteArray (lNode);
		memAssert ();
	}
	
	/* Destroy the memory */
	memset (lNode->fMemory, 0xAFAFAFAF, lNode->fSize);

	free (lNode);

}

void		
memAllowAllCurrentLeaks ()
{
	struct memInfoNode* lPtr;
	int i = 0;
	for (i = 0; i < hash_Prime; ++i) {
		lPtr = memMemoryTable.fTable[i].fHead;
		while (lPtr) {
			lPtr->fIsAllowed = 1;
			lPtr = lPtr->fNext;
		}
	}
}

void						
memReportMemLeaks ()
{
	struct memInfoNode* lPtr, *lArray;
	int lNumberOfLeaks = 0;
	int lIndex = 0;
	int i = 0;

	for (i = 0; i < hash_Prime; ++i) {
		lPtr = memMemoryTable.fTable[i].fHead;
		while (lPtr) {
			if (lPtr->fIsAllowed == 0) {
				++lNumberOfLeaks;
			}
			lPtr = lPtr->fNext;
		}
	}
	if (lNumberOfLeaks == 0) {
		return;
	}
	lArray = (struct memInfoNode*) (malloc (sizeof (struct memInfoNode) * lNumberOfLeaks));
	for (i = 0; i < hash_Prime; ++i) {
		lPtr = memMemoryTable.fTable[i].fHead;
		while (lPtr) {
			if (lPtr->fIsAllowed == 0) {
				lArray[lIndex] = *lPtr;
				if (lIndex >= lNumberOfLeaks) {
					memAssert ();
				}
				++lIndex;
			}
			lPtr = lPtr->fNext;
		}
	}
	memSortMemLeaks (lArray, lNumberOfLeaks);
	for (i = 0; i < lNumberOfLeaks; ++i) {
		memPrintLeak (&lArray[i]);
	}
	free (lArray);
}

void						
memFreeMemLeaks ()
{
	memTableDeleteAll (&memMemoryTable);
}

void						
memClearMemLeaks ()
{
	memTableClear (&memMemoryTable);
}
