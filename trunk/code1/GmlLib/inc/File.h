#pragma once
#include "compat.h"

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

			bool			Create(char *name);
			bool			OpenRead(char *name);
			bool			OpenReadWrite(char *name,bool append=false);
			void			Close();
			UInt32			GetFileSize();
			UInt32			GetFilePos();
			bool			SetFilePos(UInt32 pos);
			bool			Read(void *Buffer,UInt32 size,UInt32 *readSize=NULL);
			bool			Write(void *Buffer,UInt32 size,UInt32 *writeSize=NULL);
		};
	}
}
