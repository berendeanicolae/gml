#ifndef __CONNECTOR_UniqueFeatureConnector__
#define __CONNECTOR_UniqueFeatureConnector__

#include "gmllib.h"

class UniqueFeatureConnector : public GML::ML::IConnector
{	
	bool					OnInitConnectionToConnector();
	bool					OnInitConnectionToDataBase();
public:
	UniqueFeatureConnector();
	~UniqueFeatureConnector();

	bool					Save(char *fileName);
	bool					Load(char *fileName);
		
	bool					GetRecordLabel( double &label,UInt32 index );
	bool					GetRecord( GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask=0 );
	bool					GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index);
	bool					GetFeatureName(GML::Utils::GString &str,UInt32 index);
	
	bool					CreateMlRecord( GML::ML::MLRecord &record );
	bool					FreeMLRecord( GML::ML::MLRecord &record );
	
	bool					Close();
};

#endif
