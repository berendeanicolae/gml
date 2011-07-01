#include "gmllib.h"

struct IndexBitCounter
{
	UInt32		countInt8;
	UInt32		countInt16;
	UInt32		countInt32;
	UInt32		count254BaseInt8;
	UInt32		count254BaseInt16;
	UInt32		count253BaseInt8;
	UInt32		count253BaseInt16;	
	UInt32		maxIndex;
};

class IndexBitConnector: public GML::ML::IConnector
{
	enum
	{
		METHOD_INT8_INDEX = 0,
		METHOD_INT16_INDEX,
		METHOD_INT32_INDEX,
		METHOD_254_BASE_INDEX,
		METHOD_253_BASE_INDEX
	};
	UInt32				Method;
	UInt64				MemToAlloc;
	UInt8				*Data;	
	UInt32				*Indexes;	
	GML::Utils::BitSet	Labels;

	void				Update(IndexBitCounter &ibt,UInt32 index);
	void				ComputeMemory(IndexBitCounter &ibt,UInt64 &memory);
	bool				AddIndex(UInt32 index,UInt64 &poz);
	bool				OnInitConnectionToDataBase();
	bool				OnInitConnectionToConnector();
	bool				AllocMemory(UInt64 memory);
public:
	IndexBitConnector();

	
	bool		Close();
	bool		Save(char *fileName);
	bool		Load(char *fileName);
	bool		CreateMlRecord (GML::ML::MLRecord &record);
	bool		GetRecord(GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask=0);
	bool		FreeMLRecord(GML::ML::MLRecord &record);
	bool		GetRecordLabel(double &label,UInt32 index);


};
