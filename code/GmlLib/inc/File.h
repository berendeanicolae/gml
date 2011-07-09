#pragma once
#include "compat.h"
#include "GString.h"

class GString;
namespace GML
{
	namespace Utils
	{
		class EXPORT File
		{
			FILE_HANDLE		hFile;
		public:
			File(void);
			~File(void);

			bool			Create(char *name,bool share=false);			
			bool			OpenRead(char *name,bool share=false);
			bool			OpenReadWrite(char *name,bool append=false,bool share=false);
			void			Close();
			UInt64			GetFileSize();
			bool			GetFilePos(UInt64 &pos);
			bool			SetFilePos(UInt64 pos);
			bool			Read(void *Buffer,UInt32 size,UInt32 *readSize=NULL);
			bool			Write(void *Buffer,UInt32 size,UInt32 *writeSize=NULL);
			bool			ReadNextLine(GString &line,bool skipEmpyLines=true);
			bool			Flush();
		};
	}
}
