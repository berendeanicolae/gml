#pragma once


struct GLZDict
{
	unsigned int	Prefix;
	unsigned char	Value;
	int				NextIndex;
};

class GLZ
{
	GLZDict			*Dict;
	unsigned int	DictAllocated;
	unsigned int	DictSize,arraySize;
	unsigned char	*DecodeBuffer;
	int				*pDict;
	unsigned int	DecodeBufferSize;
	unsigned int	DecodeBufferAllocated;
	

	bool			Grow(unsigned int newSize);
	bool			Add(unsigned int Prefix,unsigned char Value);
	int				Find(unsigned int Prefix,unsigned char Value);
	bool			AddToDecodeBuffer(unsigned char ch);
	bool			UnpackCodeToDecodeBuffer(unsigned int code);

public:
	GLZ(unsigned int initialDictSize=1024,unsigned int dictMemorySize = 0xFFFFFF);
	~GLZ(void);

	bool			Compress(unsigned char *Buffer,unsigned int BufferSize,unsigned char *CompressedBuffer,unsigned int &CompressedBufferSize);
	bool			Decompress(unsigned char *CompressedBuffer,unsigned int CompressedBufferSize,unsigned char *UncompressedBuffer,unsigned int &UncompressedBufferSize);
};

