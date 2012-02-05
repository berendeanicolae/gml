#ifndef __PVM_MACROS_H_
#define __PVM_MACROS_H_

#include <cstring>
#include <cstdlib>

#define PVM_DEBUG_MODE 1

#define INFOMSG(...) { {char z123[4096]; sprintf_s(z123,4096, __VA_ARGS__) ;notif->Info("[%s] -> %s",ObjectName, z123);} };
#define INFOTHDMSG(...) { {char z123[4096]; sprintf_s(z123,4096, __VA_ARGS__) ;notif->Info("[%s] [Th:%d] -> %s",ObjectName, thData.ThreadID, z123);} };
#define ERRORMSG(...){ {char z123[4096]; sprintf_s(z123,4096, __VA_ARGS__) ;notif->Error("[%s] -> %s",ObjectName, z123);} };

#define NULLCHECKMSG(val, ...){ if (val==NULL) { ERRORMSG(__VA_ARGS__); return false; } };    
#define NULLCHECK(val) {if (val==NULL) { return false; }; };

#define CHECKMSG(val, ...){ if (val==false) { ERRORMSG(__VA_ARGS__); return false; }; };
#define CHECK(val){ if (val==false) return false;}; 

#ifdef PVM_DEBUG_MODE
	#define DBG_NULLCHECKMSG    NULLCHECKMSG
	#define DBG_NULLCHECK       NULLCHECK
	#define DBG_CHECKMSG        CHECKMSG
	#define DBG_CHECK           CHECK

	#define DBGSTOP_NULLCHECKMSG(val,...)    { if (val==NULL) {ERRORMSG(__VA_ARGS__); DebugBreak();} }
	#define DBGSTOP_NULLCHECK(val,...)       { if (val==NULL) DebugBreak(); } 
	#define DBGSTOP_CHECKMSG(val,...)        { if (val==TRUE) {ERRORMSG(__VA_ARGS__); DebugBreak();} }
	#define DBGSTOP_CHECK(val,...)           { if (val==TRUE) DebugBreak(); } 
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
