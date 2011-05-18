#include "StdAfx.h"
#include "GLZ.h"

unsigned int static LZ_TABLE[] = {
	0,
	0x1,		0x3,		0x7,		0xF,
	0x1F,		0x3F,		0x7F,		0xFF,
	0x1FF,		0x3FF,		0x7FF,		0xFFF,
	0x1FFF,		0x3FFF,		0x7FFF,		0xFFFF,
	0x1FFFF,	0x3FFFF,	0x7FFFF,	0xFFFFF,
	0x1FFFFF,	0x3FFFFF,	0x7FFFFF,	0xFFFFFF,
	0x1FFFFFF,	0x3FFFFFF,	0x7FFFFFF,	0xFFFFFFF,
	0x1FFFFFFF,	0x3FFFFFFF,	0x7FFFFFFF
};

#define HASH(prefix,value)	(((prefix<<8)|value)%arraySize)


GLZ::GLZ(unsigned int initialDictSize,unsigned int dictMemorySize)
{
	Dict = NULL;
	DictAllocated = 0;
	DictSize = 0;
	DecodeBufferSize = 0;
	DecodeBufferAllocated = 0;
	DecodeBuffer = NULL;
	arraySize = 0;
	pDict = NULL;

	if (initialDictSize>=8)
	{
		if ((Dict = new GLZDict[initialDictSize])!=NULL)
			DictAllocated = initialDictSize;
	}
	if (dictMemorySize>=0xFFFF)
	{
		if ((pDict = new int [dictMemorySize])!=NULL)
			arraySize = dictMemorySize;
	}
}
GLZ::~GLZ(void)
{
	if (Dict)
		delete Dict;
	if (DecodeBuffer)
		delete DecodeBuffer;
	if (pDict)
		delete pDict;
	Dict = NULL;
	DecodeBuffer = NULL;
	pDict = NULL;
	arraySize = 0;
	DictAllocated = 0;
	DictSize = 0;
	DecodeBufferSize = 0;
	DecodeBufferAllocated = 0;
}

bool GLZ::Grow(unsigned int newSize)
{
	GLZDict* newDict;

	if (newSize<=DictAllocated)
		return false;
	if ((newDict = new GLZDict[newSize])==NULL)
		return false;
	memcpy(newDict,Dict,sizeof(GLZDict)*DictSize);
	delete Dict;
	Dict = newDict;
	DictAllocated = newSize;
	return true;
}
bool GLZ::Add(unsigned int Prefix,unsigned char Value)
{
	GLZDict			*Entry;
	unsigned int	h = HASH(Prefix,Value);
	int				index;
	
	if (DictSize>=DictAllocated)
	{
		if (Grow(DictAllocated * 2)==false)
			return false;
	}


	Entry = &Dict[DictSize];
	Entry->Prefix = Prefix;
	Entry->Value = Value;
	Entry->NextIndex = -1;

	index = pDict[h];
	if (index==-1)
	{
		pDict[h]=DictSize;
	} else {
		// gasesc ultimul
		Entry = &Dict[index];
		while (Entry->NextIndex!=-1)
		{
			Entry = &Dict[Entry->NextIndex];
		}
		Entry->NextIndex = DictSize;
		//printf("Coliziune: %08X , %02X => %08X => [%08X,%02X]\n",Prefix,Value,h,Dict[pDict[h]].Prefix,Dict[pDict[h]].Value);
	}
	DictSize++;
	return true;
}
int	 GLZ::Find(unsigned int Prefix,unsigned char Value)
{
	GLZDict	*Entry;
	int		index = pDict[HASH(Prefix,Value)];
	
	if (index==-1)
		return -1;
	// parcurg lista
	do
	{
		Entry = &Dict[index];
		if ((Entry->Prefix==Prefix) && (Entry->Value==Value))
			return index;
		index = Entry->NextIndex;
	} while (index!=-1);
	return -1;
	/*
	GLZDict			*Entry;
	unsigned int	tr;

	for (tr=0,Entry=Dict;tr<DictSize;tr++,Entry++)
	{
		if ((Entry->Prefix == Prefix) && (Entry->Value==Value))
			return (int)tr;
	}

	return -1;
	//*/
}
bool GLZ::AddToDecodeBuffer(unsigned char ch)
{
	if (DecodeBufferSize<DecodeBufferAllocated)
	{
		DecodeBuffer[DecodeBufferSize] = ch;
		DecodeBufferSize++;
		return true;
	}
	return false;
}
bool GLZ::UnpackCodeToDecodeBuffer(unsigned int code)
{
	GLZDict *temp;

	while (code > 255)
	{
		if (code>=(DictSize+256))
			return false;
		temp = &Dict[code-256];
		if (AddToDecodeBuffer(temp->Value)==false)
			return false;
		code = temp->Prefix;
	}
	return AddToDecodeBuffer((unsigned char)code);
}
bool GLZ::Compress(unsigned char *Buffer,unsigned int BufferSize,unsigned char *CompressedBuffer,unsigned int &CompressedBufferSize)
{
	unsigned int	prefix,temp,writeSize,pValue,tr;
	unsigned char	MaxBits,TotalBits;
	int				pos;

	if ((Buffer==NULL) || (BufferSize<2) || (CompressedBuffer==NULL) || (CompressedBufferSize==0) || (Dict==NULL) || (pDict==NULL))
		return false;
	prefix = (unsigned int)(*Buffer);
	Buffer++;
	BufferSize--;
	
	MaxBits = 9;
	TotalBits = 0;
	DictSize = 0;
	writeSize = 0;
	temp = 0;
	
	for (tr=0;tr<arraySize;tr++)
		pDict[tr]=-1;

	while (BufferSize>0)
	{
		//if ((BufferSize%10000)==0) printf("%d -> DSize:%d\n",BufferSize,DictSize);
		pos = Find(prefix,*Buffer);
		if (pos==-1)
		{
			if (Add(prefix,*Buffer)==false)
				return false;
			pValue = DictSize+255;
			for (tr=9;tr<32;tr++)
			{
				MaxBits = (unsigned char)tr;
				if (pValue<=LZ_TABLE[tr])
					break;
			}
			//if (MaxBits<9)
			//	MaxBits = 9;

			temp |= (unsigned int)(prefix << (32 - MaxBits - TotalBits));
			TotalBits+=MaxBits;
			while (TotalBits>=8)
			{
				if (writeSize>=CompressedBufferSize)
					return false;
				(*CompressedBuffer) = (unsigned char)(temp >> 24);
				temp <<= 8;
				TotalBits -=8;
				writeSize++;
				CompressedBuffer++;
			}
			prefix = (*Buffer);
		} else {
			prefix = (unsigned int)pos+256;
		}
		Buffer++;
		BufferSize--;
	}

	for (tr=0;tr<3;tr++)
	{
		switch (tr)
		{
			case 0: 
				temp |= (unsigned int)(prefix << (32 - MaxBits - TotalBits));
				break;
			case 1:
				temp |= (unsigned int)(LZ_TABLE[MaxBits] << (32 - MaxBits - TotalBits));
				break;
		};
		
		TotalBits+=MaxBits;
		while (TotalBits>=8)
		{
			if (writeSize>=CompressedBufferSize)
				return false;
			(*CompressedBuffer) = (unsigned char)(temp >> 24);
			temp <<= 8;
			TotalBits -=8;
			writeSize++;
			CompressedBuffer++;
		}
	}
	CompressedBufferSize = writeSize;
	return true;
}
bool GLZ::Decompress(unsigned char *CompressedBuffer,unsigned int CompressedBufferSize,unsigned char *UncompressedBuffer,unsigned int &UncompressedBufferSize)
{
	unsigned int	prefix,next,temp,writeSize,pValue,tr;
	unsigned char	MaxBits,TotalBits,car;

	if ((UncompressedBuffer==NULL) || (UncompressedBufferSize<2) || (CompressedBuffer==NULL) || (CompressedBufferSize==0) || (Dict==NULL)  || (pDict==NULL))
		return false;
	if (DecodeBuffer!=NULL)
		delete DecodeBuffer;
	if ((DecodeBuffer = new unsigned char [UncompressedBufferSize])==NULL)
		return false;
	DecodeBufferAllocated = UncompressedBufferSize;
	DecodeBufferSize = 0;
	MaxBits = 9;
	TotalBits = 0;
	DictSize = 0;
	writeSize = 0;
	temp = 0;

	for (tr=0;tr<arraySize;tr++)
		pDict[tr]=-1;

	// read first prefix
	while ((TotalBits <= 24) && (CompressedBufferSize>0))
	{
		temp |= (unsigned int)( (unsigned int)(*CompressedBuffer) << (24 - TotalBits));
		CompressedBuffer++;
		CompressedBufferSize--;
		TotalBits += 8;
	}
	prefix = temp >> (32 - MaxBits);
	temp <<= MaxBits;
	TotalBits -= MaxBits;
	(*UncompressedBuffer) = car = (unsigned char)prefix;	
	UncompressedBuffer++;
	writeSize++;
	

	while (true)
	{
		// read next
		if (CompressedBufferSize>0)
		{
			while ((TotalBits <= 24) && (CompressedBufferSize>0))
			{
				temp |= (unsigned int)( ((unsigned int)(*CompressedBuffer)) << (24 - TotalBits));
				CompressedBuffer++;
				CompressedBufferSize--;
				TotalBits += 8;
			}
			next = temp >> (32 - MaxBits);
			temp <<= MaxBits;
			TotalBits -= MaxBits;
		} else {
			if ((temp == 0) && (TotalBits == 0))	
			{
				next = LZ_TABLE[MaxBits];
			} else {
				next = temp >> (32 - MaxBits);
				temp <<= MaxBits;
				TotalBits -= MaxBits;
			}
		}
		if (next == LZ_TABLE[MaxBits])
			break;
		if (next>=DictSize+256)
		{
			if (AddToDecodeBuffer(car)==false)
				return false;
			if (UnpackCodeToDecodeBuffer(prefix)==false)
				return false;
		} else {
			if (UnpackCodeToDecodeBuffer(next)==false)
				return false;
			if (DecodeBufferSize==0)
				return false;
			car = DecodeBuffer[DecodeBufferSize-1];
		}
		// decomprim
		while (DecodeBufferSize>0)
		{
			if (writeSize>=UncompressedBufferSize)
				return false;
			(*UncompressedBuffer) = DecodeBuffer[DecodeBufferSize-1];
			writeSize++;
			UncompressedBuffer++;
			DecodeBufferSize--;
		}

		if (Add(prefix,car)==false)
			return false;
		pValue = DictSize+257;
		for (tr=9;tr<32;tr++)
		{
			MaxBits = (unsigned char)tr;
			if (pValue<=LZ_TABLE[tr])
				break;
		}
		//if (MaxBits<9)
		//	MaxBits = 9;
		prefix = next;
	}
	UncompressedBufferSize = writeSize;
	delete DecodeBuffer;
	DecodeBuffer = NULL;
	DecodeBufferSize = 0;
	DecodeBufferAllocated = 0;
	return true;
}