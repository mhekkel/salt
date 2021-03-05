/*	
	Copyright (c) Bas Vodde.
*/

/* Include for size_t */
#include "stddef.h"

/* Hook functions for hooking in the allocate and de-allocate */
#define memNeededSpace(a) ((a) + 3)

#ifdef __cplusplus
extern "C" {
#endif

void		memLeakDetectorAlloc (void* inMemory, size_t inSize, const char* inFilename, long inLineNum, int inArray);
void		memLeakDetectorFree (void* inMemory, int inArray);

/* Hooks for output */
void		memInstallPrintHook (void (*inPrintHook) (const char*));
void		memInstallAssertHook (void (*inAssertHook) ());

/* Reporting and cleanup */
void		memAllowAllCurrentLeaks ();
void		memReportMemLeaks ();
void		memFreeMemLeaks ();
void		memClearMemLeaks ();

#ifdef __cplusplus
}
#endif
