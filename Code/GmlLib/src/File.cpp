#include "StdAfx.h"
#include "File.h"

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
bool GML::Utils::File::Create(char *name)
{
	Close();
#ifdef OS_WINDOWS
	return (bool)((hFile = CreateFileA(name,GENERIC_WRITE|GENERIC_READ,0,NULL,CREATE_ALWAYS,0,NULL))!=INVALID_HANDLE_VALUE);
#endif
	return false;
}
bool GML::Utils::File::OpenRead(char *name)
{
	Close();
#ifdef OS_WINDOWS
	return (bool)((hFile = CreateFileA(name,GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL))!=INVALID_HANDLE_VALUE);
#endif
	return false;
}
bool GML::Utils::File::OpenReadWrite(char *name,bool append)
{
	Close();
#ifdef OS_WINDOWS
	return (bool)((hFile = CreateFileA(name,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL))!=INVALID_HANDLE_VALUE);
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
UInt32 GML::Utils::File::GetFileSize()
{
#ifdef OS_WINDOWS
	if (hFile!=INVALID_HANDLE_VALUE)
		return (UInt32)::GetFileSize(hFile,NULL);
#endif
	return 0;
}
UInt32 GML::Utils::File::GetFilePos()
{
#ifdef OS_WINDOWS
	if (hFile==INVALID_HANDLE_VALUE)
		return 0;
	else
		return (UInt32)SetFilePointer(hFile,0,NULL,FILE_CURRENT);
#endif
}
bool GML::Utils::File::SetFilePos(UInt32 pos)
{
#ifdef OS_WINDOWS
	if (hFile==INVALID_HANDLE_VALUE)
		return false;
	else
		return (SetFilePointer(hFile,(LONG)pos,NULL,FILE_BEGIN)==pos);
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