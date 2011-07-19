#ifndef __CONNECTOR_NameConvertConnector__
#define __CONNECTOR_NameConvertConnector__

#include "gmllib.h"

class NameConvertConnector : public GML::ML::IConnector
{	
	GML::Utils::GString		FeatureNamesList;
	GML::Utils::GString*	NewNames;
	bool					OnInitConnectionToConnector();
	
public:
	NameConvertConnector();
			
	bool					GetRecordLabel( double &label,UInt32 index );
	bool					GetRecord( GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask=0 );
	bool					GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index);
	bool					GetFeatureName(GML::Utils::GString &str,UInt32 index);
	
	bool					CreateMlRecord( GML::ML::MLRecord &record );
	bool					FreeMLRecord( GML::ML::MLRecord &record );
	
	bool					Close();
};

#endif
