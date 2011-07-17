#include "CacheFile.h"

#define GET_FUNCTION(name,tip) bool GML::Utils::CacheFile::name (UInt64 pos,tip &value) { void *data; if ((data=GetSafePointer(pos,sizeof(tip)))==NULL) return false;value = *(tip *)data;return true;}
#define READ_FUNCTION(name,tip) bool GML::Utils::CacheFile::name (UInt64 &pos,tip &value) { void *data; if ((data=GetSafePointer(pos,sizeof(tip)))==NULL) return false;value = *(tip *)data;pos+=sizeof(tip);return true;}
//======================================================================================
GML::Utils::CacheFile::CacheFile()
{
	Cache = NULL;
	CacheAllocSize = 0;
	CacheStart = CacheEnd = FileSize = 0;
}
GML::Utils::CacheFile::~CacheFile()
{
	Close();
}
bool GML::Utils::CacheFile::UpdateCache(UInt64	newPoz)
{
	UInt64	read;
	if (Cache==NULL)
		return false;
	if (file.SetFilePos(newPoz)==false)
		return false;
	if (file.Read(Cache,CacheAllocSize,&read)==false)
		return false;
	CacheStart = newPoz;
	CacheEnd = newPoz+read;
	return true;
}
void*GML::Utils::CacheFile::GetSafePointer(UInt64 pos,UInt32 size)
{
	if ((pos<CacheStart) || (pos+size>CacheEnd))
	{
		if (UpdateCache(pos)==false)
			return NULL;
		if ((pos<CacheStart) || (pos+size>CacheEnd))
			return NULL;		
	} 
	return &Cache[(UInt32)(pos-CacheStart)];
}
bool GML::Utils::CacheFile::Open(char *fileName,UInt32 _CacheAllocSize)
{
	Close();

	if (_CacheAllocSize<32)
		return false;
	if ((Cache = new UInt8[_CacheAllocSize])==NULL)
		return false;	
	CacheAllocSize = _CacheAllocSize;
	while (true)
	{
		if (file.OpenRead(fileName,true)==false)
			break;
		if ((FileSize = file.GetFileSize())==0)
			break;
		if (UpdateCache(0)==false)
			break;
		return true;
	}
	Close();
	return false;
}
void GML::Utils::CacheFile::Close()
{
	file.Close();
	if (Cache!=NULL)
		delete Cache;
	Cache = NULL;
	CacheStart = CacheEnd = FileSize = 0;
}
UInt64 GML::Utils::CacheFile::GetFileSize()
{
	return FileSize;
}
bool GML::Utils::CacheFile::IsEOF(UInt64 pos)
{
	return (bool)(pos>=FileSize);
}
bool GML::Utils::CacheFile::GetBuffer(UInt64 pos,void *Buffer,UInt32 size)
{
	void *data;
	if ((data = GetSafePointer(pos,size))==NULL)
		return false;
	MEMCOPY(Buffer,data,size);
	return true;
}
bool GML::Utils::CacheFile::ReadBuffer(UInt64 &pos,void *Buffer,UInt32 size)
{
	void *data;
	if ((data = GetSafePointer(pos,size))==NULL)
		return false;
	MEMCOPY(Buffer,data,size);
	pos+=(UInt64)size;
	return true;
}
bool GML::Utils::CacheFile::GetLine(UInt64 pos,GML::Utils::GString &str)
{
	Int8 ch;	
	if (str.Set("")==false)
		return false;
	
	while(pos<FileSize)
	{
		if (GetInt8(pos,ch)==false)
			return false;
		if ((ch==0) || (ch=='\n') || (ch=='\r'))
			return true;
		if (str.AddChar(ch)==false)
			return false;
		pos++;
	} 
	return true;
}
bool GML::Utils::CacheFile::ReadLine(UInt64 &pos,GML::Utils::GString &str)
{
	Int8	data[2];

	if (GetLine(pos,str)==false)
		return false;
	pos+=str.Len();
	if (pos+2<=FileSize)
	{
		if (GetBuffer(pos,&data[0],2)==false)
			return false;		
		if (data[0]==0)
		{
			pos++;
		} else 
		if (data[0]=='\n')
		{
			pos++;
			if (data[1]=='\r')
				pos++;
		} else 
		if (data[0]=='\r')
		{
			pos++;
			if (data[1]=='\n')
				pos++;
		}
	} else 
	if (pos+1<=FileSize)
	{
		if (GetBuffer(pos,&data[0],1)==false)
			return false;
		if ((data[0]==0) || (data[0]=='\n') || (data[0]=='\r'))
			pos++;
	}

	return true;
}
//=========================== Template Functions ========================================
GET_FUNCTION(GetUInt8,UInt8);
GET_FUNCTION(GetUInt16,UInt16);
GET_FUNCTION(GetUInt32,UInt32);
GET_FUNCTION(GetUInt64,UInt64);
GET_FUNCTION(GetInt8,Int8);
GET_FUNCTION(GetInt16,Int16);
GET_FUNCTION(GetInt32,Int32);
GET_FUNCTION(GetInt64,Int64);
GET_FUNCTION(GetFloat,float);
GET_FUNCTION(GetDouble,double);
GET_FUNCTION(GetBool,bool);

READ_FUNCTION(ReadUInt8,UInt8);
READ_FUNCTION(ReadUInt16,UInt16);
READ_FUNCTION(ReadUInt32,UInt32);
READ_FUNCTION(ReadUInt64,UInt64);
READ_FUNCTION(ReadInt8,Int8);
READ_FUNCTION(ReadInt16,Int16);
READ_FUNCTION(ReadInt32,Int32);
READ_FUNCTION(ReadInt64,Int64);
READ_FUNCTION(ReadFloat,float);
READ_FUNCTION(ReadDouble,double);
READ_FUNCTION(ReadBool,bool);