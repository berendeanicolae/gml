#ifndef __CONNECTOR_CompressedBitConnector__
#define __CONNECTOR_CompressedBitConnector__

#include "gmllib.h"
#define INVALID_CACHE_INDEX	0xFFFFFFFFFFFFFFFF
struct IndexBitCounter
{
	UInt32		Last;
	UInt64		countValue255;
	UInt64		countInt8;
	UInt64		countInt16;
	UInt64		countInt32;
	/*
	UInt64		count254BaseInt8;
	UInt64		count254BaseInt16;
	UInt64		count253BaseInt8;
	UInt64		count253BaseInt16;	
	*/
	UInt64		count254BaseLo;
	UInt64		count254BaseHi;
	UInt64		count253BaseLo;
	UInt64		count253BaseHi;	
	UInt64		count252BaseLo;
	UInt64		count252BaseHi;	
	UInt64		count251BaseLo;
	UInt64		count251BaseHi;	
	UInt64		count250BaseLo;
	UInt64		count250BaseHi;		
	UInt64		count7Bit;
	UInt64		count15Bit;	
	UInt32		maxIndex;
};
struct CompressedBitConnectorHeader: public GML::ML::CacheHeader
{
	UInt64		MemToAlloc;
};
struct CompressedBitConnectorThreadCacheData
{
	GML::Utils::File	CacheFile;
	UInt64				CacheStart,CacheEnd;
	UInt8				Data[1];
};
class CompressedBitConnector: public GML::ML::IConnector
{
	enum
	{
		METHOD_INT8_INDEX = 0,
		METHOD_254_BASE_INDEX,
		METHOD_253_BASE_INDEX,
		METHOD_255_BASE_INDEX,
		METHOD_INT16_INDEX,
		METHOD_INT32_INDEX,
		METHOD_INT15_EXTEND_INDEX,
		METHOD_252_BASE_INDEX,
		METHOD_251_BASE_INDEX,
		METHOD_250_BASE_INDEX,	
	};
	UInt32				Method;
	UInt64				MemToAlloc;
	UInt8				*Data;	
	UInt64				*Indexes;	
	GML::Utils::BitSet	Labels;
	UInt32				CacheMemory;
	
	GML::Utils::GString	CacheFileName;

	void				Update(IndexBitCounter &ibt,UInt32 index);
	void				ComputeMemory(IndexBitCounter &ibt,UInt64 &memory);
	bool				AddIndex(UInt32 index,UInt64 &poz);
	bool				OnInitConnectionToDataBase();
	bool				OnInitConnectionToConnector();
	bool				AllocMemory(UInt64 memory);
	bool				UpdateCacheMemory(CompressedBitConnectorThreadCacheData &ibthData,UInt64 start,UInt64 szBuffer);
public:
	CompressedBitConnector();
	
	bool		Close();
	bool		Save(char *fileName);
	bool		Load(char *fileName);
	bool		CreateMlRecord (GML::ML::MLRecord &record);
	bool		GetRecord(GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask=0);
	bool		FreeMLRecord(GML::ML::MLRecord &record);
	bool		GetRecordLabel(double &label,UInt32 index);


};


#endif
