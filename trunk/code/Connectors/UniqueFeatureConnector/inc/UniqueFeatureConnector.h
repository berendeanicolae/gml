#ifndef __CONNECTOR_UniqueFeatureConnector__
#define __CONNECTOR_UniqueFeatureConnector__

#include "gmllib.h"

struct FeatInfo
{
	GML::DB::RecordHash		Hash;
	UInt32					Index;
};
class UniqueFeatureConnector : public GML::ML::IConnector
{	
	GML::Utils::GTFVector<FeatInfo>	FList;
	GML::Utils::Indexes				Indexes;


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
