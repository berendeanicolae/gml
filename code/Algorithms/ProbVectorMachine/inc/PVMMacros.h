#ifndef __PVM_MACROS_H_
#define __PVM_MACROS_H_

#include <cstring>
#include <cstdlib>

#define INFOMSG(...) { {char z123[4096]; sprintf_s(z123,4096, __VA_ARGS__) ;notif->Info("[%s] -> %s",ObjectName, z123);} };
#define INFOTHDMSG(...) { {char z123[4096]; sprintf_s(z123,4096, __VA_ARGS__) ;notif->Info("[%s] [Th:%d] -> %s",ObjectName, thData.ThreadID, z123);} };
#define ERRORMSG(...){ {char z123[4096]; sprintf_s(z123,4096, __VA_ARGS__) ;notif->Error("[%s] -> %s",ObjectName, z123);} };

#define NULLCHECKMSG(val, ...){ if (val==NULL) { ERRORMSG(__VA_ARGS__); return false; } };    

#define NULLCHECK(val) {if (val==NULL) { return false; }; };

#define CHECKMSG(val, ...){ if (val==false) { ERRORMSG(__VA_ARGS__); return false; }; };
#define CHECK(val){ if (val==false) return false;}; 

#define CLCHECK(val) { if (val!=CL_SUCCESS) return false;}
#define CLDISPLAYERR { notif->Error("[%s] -> Line: %d; OpenCL Error: %s", ObjectName, __LINE__, GetCLErrorMsg(ErrorCode));}
#define CLCHECKMSG(...) { if (ErrorCode!=CL_SUCCESS){ ERRORMSG(__VA_ARGS__); CLDISPLAYERR(ErrorCode); return false;}}


#endif //__PVM_MACROS_H_