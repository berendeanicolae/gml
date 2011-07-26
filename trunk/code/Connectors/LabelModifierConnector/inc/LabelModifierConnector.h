#ifndef __CONNECTOR_LabelModifierConnector__
#define __CONNECTOR_LabelModifierConnector__

#include "gmllib.h"

struct LabelInfo
{
	GML::DB::RecordHash		Hash;
	double					Label;
};

class LabelModifierConnector : public GML::ML::IConnector
{	
	GML::Utils::GTFVector<LabelInfo>	HashList;
	GML::Utils::GTFVector<double>		Labels;
	GML::Utils::GString					HashFileName;

	bool					OnInitConnectionToConnector();
	bool   					LoadTextHashFile();
public:
	LabelModifierConnector();
		
	bool					GetRecordLabel( double &label,UInt32 index );
	bool					GetRecord( GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask=0 );
	bool					GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index);
		
	bool					CreateMlRecord( GML::ML::MLRecord &record );
	bool					FreeMLRecord( GML::ML::MLRecord &record );
	
	bool					Close();
};

#endif
