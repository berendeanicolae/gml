#ifndef __CONNECTOR_LabelFilterConnector__
#define __CONNECTOR_LabelFilterConnector__

#include "gmllib.h"

class LabelFilterConnector : public GML::ML::IConnector
{	
	enum
	{
		OP_EQUAL = 0,
		OP_DIFFERENT,
		OP_BIGGER,
		OP_SMALLER,
		OP_BIGGER_EQ,
		OP_SMALLER_EQ,
	};
	GML::Utils::Indexes		Indexes;
	UInt32					Operation;
	double					Value;

	bool					OnInitConnectionToConnector();
public:
	LabelFilterConnector();

		
	bool					GetRecordLabel( double &label,UInt32 index );
	bool					GetRecord( GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask=0 );
	bool					GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index);
	bool					CreateMlRecord( GML::ML::MLRecord &record );
	bool					FreeMLRecord( GML::ML::MLRecord &record );
	bool					Close();
};

#endif
