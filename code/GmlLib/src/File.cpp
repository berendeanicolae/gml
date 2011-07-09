#include "File.h"

#define MAX_LINE_BUFFER_SIZE	512

GML::Utils::File::File(void)
{
#ifdef OS_WINDOWS
	hFile = INVALID_HANDLE_VALUE;
#endif
}

GML::Utils::File::~File(void)
{
	Close();
}
bool GML::Utils::File::Create(char *name,bool share)
{
	Close();
#ifdef OS_WINDOWS
	DWORD	shareFlags = 0;
	if (share)
		shareFlags = FILE_SHARE_READ;
	return (bool)((hFile = CreateFileA(name,GENERIC_WRITE|GENERIC_READ,share,NULL,CREATE_ALWAYS,0,NULL))!=INVALID_HANDLE_VALUE);
#endif
	return false;
}
bool GML::Utils::File::OpenRead(char *name,bool share)
{
	Close();
#ifdef OS_WINDOWS
	DWORD	shareFlags = 0;
	if (share)
		shareFlags = FILE_SHARE_READ;
	return (bool)((hFile = CreateFileA(name,GENERIC_READ,share,NULL,OPEN_EXISTING,0,NULL))!=INVALID_HANDLE_VALUE);
#endif
	return false;
}
bool GML::Utils::File::OpenReadWrite(char *name,bool append,bool share)
{
	Close();
#ifdef OS_WINDOWS
	DWORD	shareFlags = 0;
	if (share)
		shareFlags = FILE_SHARE_READ;
	return (bool)((hFile = CreateFileA(name,GENERIC_READ|GENERIC_WRITE,share,NULL,OPEN_EXISTING,0,NULL))!=INVALID_HANDLE_VALUE);
#endif
	return false;
}
void GML::Utils::File::Close()
{
#ifdef OS_WINDOWS
	if (hFile!=INVALID_HANDLE_VALUE)
		CloseHandle(hFile);
	hFile = INVALID_HANDLE_VALUE;
#endif
}
UInt64 GML::Utils::File::GetFileSize()
{
#ifdef OS_WINDOWS
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER	li;
		if (::GetFileSizeEx(hFile,&li)==FALSE)
			return 0;
		return (UInt64)li.QuadPart;
	}
#endif
	return 0;
}
bool GML::Utils::File::GetFilePos(UInt64 &pos)
{
#ifdef OS_WINDOWS
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER	li,temp;
		temp.QuadPart = 0;
		if (SetFilePointerEx(hFile,temp,&li,FILE_CURRENT)==FALSE)
			return false;
		pos = li.QuadPart;
		return true;
	}
#endif
	return false;
}
bool GML::Utils::File::SetFilePos(UInt64 pos)
{
#ifdef OS_WINDOWS
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER	li;
		li.QuadPart = pos;
		if (SetFilePointerEx(hFile,li,NULL,FILE_BEGIN)==FALSE)
			return false;
		return true;
	}
#endif
	return false;
}
bool GML::Utils::File::Read(void *Buffer,UInt32 size,UInt32 *readSize)
{
#ifdef OS_WINDOWS
	DWORD	nrRead;
	if ((hFile==INVALID_HANDLE_VALUE) || (Buffer==NULL))
		return false;
	if (size==0)
		return true;
	if (!ReadFile(hFile,Buffer,size,&nrRead,NULL))
		return false;
	if (readSize!=NULL)
	{
		(*readSize) = (UInt32)nrRead;
		return true;
	}
	return (bool)((UInt32)nrRead==size);
#endif
	return false;
}
bool GML::Utils::File::Write(void *Buffer,UInt32 size,UInt32 *writeSize)
{
#ifdef OS_WINDOWS
	DWORD	nrWrite;
	if ((hFile==INVALID_HANDLE_VALUE) || (Buffer==NULL))
		return false;
	if (size==0)
		return true;
	if (!WriteFile(hFile,Buffer,size,&nrWrite,NULL))
		return false;
	if (writeSize!=NULL)
	{
		(*writeSize) = (UInt32)nrWrite;
		return true;
	}
	return (bool)((UInt32)nrWrite==size);
#endif
	return false;
}
bool GML::Utils::File::ReadNextLine(GString &line,bool skipEmpyLines)
{
	char	temp[MAX_LINE_BUFFER_SIZE];
	UInt64	cPos;
	UInt32	sizeRead,tr;	
	bool	foundEOL;

	if (line.Set("")==false)
		return false;
	if (GetFilePos(cPos)==false)
		return false;

	while (true)
	{
		foundEOL = false;
		if (Read(temp,MAX_LINE_BUFFER_SIZE,&sizeRead)==false)
			return false;
		if (sizeRead==0)
			return false;
		for (tr=0;(tr<sizeRead) && (temp[tr]!='\n') && (temp[tr]!='\r');tr++,cPos++)
		{
			if (temp[tr]!=0)
			{
				if (line.AddChar(temp[tr])==false)
					return false;
			}
		}
		if ((temp[tr]=='\n') || (temp[tr]=='\r'))
		{
			cPos++;
			foundEOL = true;
			if ((temp[tr]=='\n') && (tr+1<sizeRead) && (temp[tr+1]=='\r'))
				cPos++;
			if ((temp[tr]=='\r') && (tr+1<sizeRead) && (temp[tr+1]=='\n'))
				cPos++;			
		}
		if (SetFilePos(cPos)==false)
			return false;
		if ((tr==MAX_LINE_BUFFER_SIZE) && (foundEOL==false))
			continue;
		if ((skipEmpyLines) && (line.Len()==0))
			continue;
		break;
	}
	return true;
}
bool GML::Utils::File::Flush()
{
#ifdef OS_WINDOWS
	if (hFile==INVALID_HANDLE_VALUE) 
		return false;
	return (bool)FlushFileBuffers(hFile);
#endif
}