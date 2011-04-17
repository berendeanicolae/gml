#ifndef __COMPAT_H
#define __COMPAT_H

#define NOTIFYER_EXT		".ntf"
#define DATABASE_EXT		".db"
#define CONNECTOR_EXT		".dbc"
#define ALGORITHM_EXT		".alg"

#define NOTIFYER_FOLDER		"Notifiers"
#define DATABASE_FOLDER		"DataBases"
#define CONNECTOR_FOLDER	"Connectors"
#define ALGORITHM_FOLDER	"Algorithms"

#ifdef _DEBUG
	#define DEBUGMSG	printf
#else
	#define DEBUGMSG	
#endif

#ifdef WIN32
	#define OS_WINDOWS
#endif

#ifdef OS_WINDOWS
    #ifdef OS_UNIX
        #undef OS_UNIX
    #endif
#endif

#ifdef OS_WINDOWS
	#include "windows.h"
	#include <string>

	#define UInt8				unsigned char
	#define UInt16				unsigned short
	#define UInt32				unsigned int
	#define UInt64				unsigned __int64

	#define Int8				char
	#define Int16				short
	#define Int32				int
	#define Int64				__int64

	#define FILE_HANDLE			HANDLE

	#define memcpy				memcpy
	#define memmove				memmove
	#define memcmp				memcmp
	#define free				free
	#define MEMCOPY				memcpy
	#define MEMSET				memset

	#define EXPORT				_declspec(dllexport)

#endif

#ifdef OS_UNIX
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <stdint.h>
    #include <stddef.h>
    #include <unistd.h>

	#define UInt8				uint8_t
	#define UInt16				uint16_t
	#define UInt32				uint32_t
	#define UInt64				uint64_t

	#define FILE_HANDLE				int
	#define INVALID_HANDLE_VALUE	-1
	#define GENERIC_ERROR			-1

	#define memcpy				memcpy
	#define memmove				memmove
	#define memcmp				memcmp
	#define free				free
#endif

#define LIB_INTERFACE(tip,author,version,description)\
	BOOL APIENTRY DllMain( HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved) { return TRUE; }; \
	extern "C" EXPORT void* CreateInterface() { return new tip (); };\
	extern "C" EXPORT char*	GetInterfaceAuthor() { return (char*)author; };\
	extern "C" EXPORT char*	GetInterfaceDescription() { return (char*)description; };\
	extern "C" EXPORT int	GetInterfaceVersion() { return (int)version; };\
	extern "C" EXPORT bool	GetInterfaceProperty(GML::Utils::AttributeList &attr)\
	{														\
		tip* t = new tip();									\
		if (t!=NULL)										\
			return t->GetProperty(attr);					\
		return false;										\
	};														\
	extern "C" EXPORT bool	SaveInterfaceTemplate(char* fileName)	\
	{																\
		GML::Utils::AttributeList attr;								\
		tip* t = new tip();											\
		if (t!=NULL) {												\
			if (t->GetProperty(attr))								\
				return attr.Save(fileName);							\
			}														\
		return false;												\
	};															
	

#endif