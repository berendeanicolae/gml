#ifndef __CONNECTOR_UniqueFeatureConnector__
#define __CONNECTOR_UniqueFeatureConnector__

#include "gmllib.h"

struct FeatInfo
{
	GML::DB::RecordHash		Hash;
	UInt32					Index;
};
struct FeatCountInfo
{
	UInt32					TotalPoz,TotalNeg;
	UInt32					UniqPoz,UniqNeg;
	UInt32					CombPoz,CombNeg;
};
class UniqueFeatureConnector : public GML::ML::IConnector
{	
	enum
	{
		ACTION_KEEP = 0,
		ACTION_REMOVE,

		ACTION_KEEP_ONE = 0,
		ACTION_KEEP_ALL,
		ACTION_REMOVE_ALL,

		ACTION_MC_KEEP_ALL = 0,
		ACTION_MC_REMOVE_ALL,
		ACTION_MC_KEEP_FIRST_POSITIVE,
		ACTION_MC_KEEP_FIRST_NEGATIVE,
		ACTION_MC_KEEP_FIRST_POSITIVE_AND_NEGATIVE,
		ACTION_MC_KEEP_ONLY_POSITIVE,
		ACTION_MC_KEEP_ONLY_NEGATIVE,

	};


	GML::Utils::GTFVector<FeatInfo>	FList;
	GML::Utils::Indexes				Indexes;

	FeatCountInfo					countInfo;
	
	UInt32							IfUniqeRecordPositive,IfMultipleRecordsPositive;
	UInt32							IfUniqeRecordNegative,IfMultipleRecordsNegative;
	UInt32							IsMultiClassRecords;

	bool					DoActionOnSingleClass(UInt32 start,UInt32 end,UInt32 ifOne,UInt32 ifMany);
	bool					OnInitConnectionToConnector();
	bool					AnalizeSubList(UInt32 start,UInt32 end);
	
public:
	UniqueFeatureConnector();
	~UniqueFeatureConnector();

		
	bool					GetRecordLabel( double &label,UInt32 index );
	bool					GetRecord( GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask=0 );
	bool					GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index);
	
	bool					CreateMlRecord( GML::ML::MLRecord &record );
	bool					FreeMLRecord( GML::ML::MLRecord &record );
	
	bool					Close();
};

#endif
