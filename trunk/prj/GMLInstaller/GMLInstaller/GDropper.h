#pragma once

#include "Windows.h"
#include "GString.h"

class GDropper
{
public:
	struct FileHeader
	{
		unsigned int	CompressedSize;
		unsigned int	UncompressedSize;
		unsigned int	CRC32;
		unsigned int	NameIndex;
		unsigned int	Flags;
		unsigned int	Offset;
	};
	struct GDropperHeader
	{
		unsigned int	Magic;
		unsigned int	TotalFiles;
		unsigned int	NamesSize;
		FileHeader		Files[1];
	};
private:
					
	unsigned char		*Data;
	unsigned char		*Names;
	unsigned char		*FilesData;
	unsigned int		DataSize;
	GString				Error;

public:
	GDropperHeader		*Header;
public:
	GDropper(void);
	~GDropper(void);

	bool				Load(char *fileName,unsigned int offset=0);
	char*				GetError();
	char*				GetFileName(unsigned int index);
	bool				Extract(unsigned int index,char *localFileName);

	bool				CreatePath(char *path);
	bool				PathExists(char *path);
};

