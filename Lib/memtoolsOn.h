/*	
	Copyright (c) Bas Vodde.
*/

#ifdef _MEMDEBUGDEFINE

#undef NEW
	#define NEW new (__FILE__,  __LINE__)
#endif
