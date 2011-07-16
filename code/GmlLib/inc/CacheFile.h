#ifndef __STREAM_FILE__
#define __STREAM_FILE__

#include "GString.h"
#include "File.h"

namespace GML
{
	namespace Utils
	{
		class EXPORT CacheFile
		{
			File		file;
			UInt8*		Cache;
			UInt64		CacheAllocSize;
			UInt64		CacheStart,CacheEnd,FileSize;
						
			bool		UpdateCache(UInt64 newPoz);
			 
		public:
			CacheFile();
			~CacheFile();

			bool		Open(char *fileName,UInt32 CacheAllocSize = 0xFFFF);
			void		Close();			
			UInt64		GetFileSize();
			void*		GetSafePointer(UInt64 pos,UInt32 size);

			// Get
			bool		GetUInt8(UInt64 pos,UInt8 &value);
			bool		GetUInt16(UInt64 pos,UInt16 &value);
			bool		GetUInt32(UInt64 pos,UInt32 &value);
			bool		GetUInt64(UInt64 pos,UInt64 &value);
			bool		GetInt8(UInt64 pos,Int8 &value);
			bool		GetInt16(UInt64 pos,Int16 &value);
			bool		GetInt32(UInt64 pos,Int32 &value);
			bool		GetInt64(UInt64 pos,Int64 &value);
			bool		GetFloat(UInt64 pos,float &value);
			bool		GetDouble(UInt64 pos,double &value);
			bool		GetBool(UInt64 pos,bool &value);
			bool		GetBuffer(UInt64 pos,void *Buffer,UInt32 size);			
			bool		GetLine(UInt64 pos,GML::Utils::GString &line);

			// read
			bool		ReadUInt8(UInt64 &pos,UInt8 &value);
			bool		ReadUInt16(UInt64 &pos,UInt16 &value);
			bool		ReadUInt32(UInt64 &pos,UInt32 &value);
			bool		ReadUInt64(UInt64 &pos,UInt64 &value);
			bool		ReadInt8(UInt64 &pos,Int8 &value);
			bool		ReadInt16(UInt64 &pos,Int16 &value);
			bool		ReadInt32(UInt64 &pos,Int32 &value);
			bool		ReadInt64(UInt64 &pos,Int64 &value);
			bool		ReadFloat(UInt64 &pos,float &value);
			bool		ReadDouble(UInt64 &pos,double &value);
			bool		ReadBool(UInt64 &pos,bool &value);
			bool		ReadBuffer(UInt64 &pos,void *Buffer,UInt32 size);	
			bool		ReadLine(UInt64 &pos,GML::Utils::GString &line);
		};
	}
}

#endif
