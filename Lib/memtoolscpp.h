/*	
	Copyright (c) Bas Vodde.

	Interface for new and delete allocators for C++.
	_MEMDEBUG must be defined for using the operator new and delete
	_MEMDEBUGDEFINE must be defined to get extra information from the operator new
*/


/* Include for size_t */
#include "memtools.h"
#include <cstdlib>

#ifdef _MEMDEBUG

inline 
void* operator new (size_t inSize)
{
	void* lMem = malloc (memNeededSpace (inSize));
	memLeakDetectorAlloc (lMem, inSize, "No Filename", 1, 0);
	return lMem;
}

inline 
void* operator new[] (size_t inSize)	
{
	void* lMem = malloc (memNeededSpace (inSize));
	memLeakDetectorAlloc (lMem, inSize, "No Filename", 1, 1);
	return lMem;
}

inline 
void  operator delete (void* inPtr)		
{ 
	memLeakDetectorFree (inPtr, 0);
	free (inPtr);
}

inline 
void  operator delete[] (void* inPtr)	
{ 
	memLeakDetectorFree (inPtr, 1);
	free (inPtr);
}

#ifdef _MEMDEBUGDEFINE

inline 
void* operator new (size_t inSize, const char* inFilename, long inLine)		
{ 
	void* lMem = malloc (memNeededSpace (inSize));
	memLeakDetectorAlloc (lMem, inSize, inFilename, inLine, 0);
	return lMem;
}

inline 
void* operator new[] (size_t inSize, const char* inFilename, long inLine)	
{ 
	void* lMem = malloc (memNeededSpace (inSize));
	memLeakDetectorAlloc (lMem, inSize, inFilename, inLine, 1);
	return lMem;
}

inline 
void  operator delete (void* inPtr, const char* /*inFilename*/, long /*inLine*/)		
{ 
	memLeakDetectorFree (inPtr, 0);
	free (inPtr);
}

inline 
void  operator delete[] (void* inPtr, const char* /*inFilename*/, long /*inLine*/)	
{ 
	memLeakDetectorFree (inPtr, 1);
	free (inPtr);
}

#endif


#ifdef _MEMDEBUGDEFINE
	#include "memtoolson.h"
#endif
#endif
