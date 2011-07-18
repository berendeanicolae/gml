#ifndef __CONNECTOR_MergeConnector__
#define __CONNECTOR_MergeConnector__

#include "gmllib.h"

class MergeConnector : public GML::ML::IConnector
{	
	GML::Utils::Interval	*Translate;
	bool					OnInitConnectionToConnector();
	
	GML::ML::IConnector*	IndexToConnector(UInt32 &index);
public:
	MergeConnector();
	~MergeConnector();

	bool 					AllowConnectors(UInt32 count);
	
	bool					GetRecordLabel( double &label,UInt32 index );
	bool					GetRecord( GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask=0 );
	bool					GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index);
		
	bool					CreateMlRecord( GML::ML::MLRecord &record );
	bool					FreeMLRecord( GML::ML::MLRecord &record );
	
	bool					Close();
};

#endif
