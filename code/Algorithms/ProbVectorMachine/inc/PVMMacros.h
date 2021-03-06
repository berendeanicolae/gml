#ifndef __PVM_MACROS_H_
#define __PVM_MACROS_H_

#include <cstring>
#include <cstdlib>

//#define PVM_DEBUG_MODE 1
//#define MCU_DEBUG_MODE 1

#ifdef MCU_DEBUG_MODE
	#define INFOMSG(...) { {char z123[4096]; sprintf_s(z123,4096, __VA_ARGS__) ;notif->Info("[%s] -> %s",ObjectName, z123);} };
	#define INFOTHDMSG(...) { {char z123[4096]; sprintf_s(z123,4096, __VA_ARGS__) ;notif->Info("[%s] [Th:%d] -> %s",ObjectName, thData.ThreadID, z123);} };
	#define ERRORMSG(...){ {char z123[4096]; sprintf_s(z123,4096, __VA_ARGS__) ;notif->Error("[%s] -> %s",ObjectName, z123);} };

	#define NULLCHECKMSG(val, ...){ if (val==NULL) { ERRORMSG(__VA_ARGS__); return false; } };    
	#define NULLCHECK(val) {if (val==NULL) { return false; }; };

	#define CHECKMSG(val, ...){ if (val==false) { ERRORMSG(__VA_ARGS__); return false; }; };
	#define CHECK(val){ if (val==false) return false;}; 

	#define ATCHECKMSG(val, ...){ if (val==false) { ERRORMSG(__VA_ARGS__); AtKillThread=true; SetEvent(AtEventWorking); return 0xffFFffFF; }; };
	#define ATCHECKMSG_INT3(val, ...){ if (val==false) { ERRORMSG(__VA_ARGS__); AtKillThread=true; SetEvent(AtEventWorking); DebugBreak(); }; };

	void* pvm_malloc_func (int val);
	void pvm_free_func (void* __ptr__);

	extern void* __ptr__;
	#define pvm_malloc(val) (__ptr__ = malloc(val)); INFOMSG("malloc-> ptr: %08x; size: %.05f MB; at file: %s line: %d", __ptr__, float(val)/(1024.0*1024.0),__FILE__, __LINE__);
	#define ptr_free(val) free(val); INFOMSG("free-> ptr: %08x; at file: %s line: %d", val, __FILE__, __LINE__);

#else
	#define INFOMSG(...)    //
	#define INFOTHDMSG(...) //
	#define ERRORMSG(...) //	

	#define NULLCHECKMSG(val, ...) { val; }
	#define NULLCHECK(val) //

	#define CHECKMSG(val, ...) { val; }
	#define CHECK(val) { val }

	#define ATCHECKMSG(val, ...) { val; }

	#pragma warning( disable : 4553 )
	#pragma warning( disable : 4552 )

	#define pvm_malloc(val) malloc(val);
	#define pvm_free(ptr) free(ptr);

#endif

#ifdef PVM_DEBUG_MODE
	#define DBG_NULLCHECKMSG    NULLCHECKMSG
	#define DBG_NULLCHECK       NULLCHECK
	#define DBG_CHECKMSG        CHECKMSG
	#define DBG_CHECK           CHECK

	#define DBGSTOP_NULLCHECKMSG(val,...)    { if (val==NULL) {ERRORMSG(__VA_ARGS__); DebugBreak();} }
	#define DBGSTOP_NULLCHECK(val,...)       { if (val==NULL) DebugBreak(); } 
	#define DBGSTOP_CHECKMSG(val,...)        { if (val==false) {ERRORMSG(__VA_ARGS__); DebugBreak();} }
	#define DBGSTOP_CHECK(val,...)           { if (val==false) DebugBreak(); } 
#else
	#define DBG_NULLCHECKMSG    //
	#define DBG_NULLCHECK       //
	#define DBG_CHECKMSG        //
	#define DBG_CHECK           //

	#define DBGSTOP_NULLCHECKMSG    //
	#define DBGSTOP_NULLCHECK       //
	#define DBGSTOP_CHECKMSG        //
	#define DBGSTOP_CHECK           //
#endif

#define  UNGIGA (1024*1024*1024)
#define  UNMEGA (1024*1024)

#endif //__PVM_MACROS_H_
