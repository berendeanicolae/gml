#include "gmllib.h"

class BitConnector: public GML::ML::IConnector
{
	UInt8				*Data;	
	UInt32				Align8Size;

	bool				OnInitConnectionToDataBase();
	bool				OnInitConnectionToConnector();
	bool				AllocMemory();
public:
	BitConnector();

	
	bool		Close();
	bool		Save(char *fileName);
	bool		Load(char *fileName);
	bool		CreateMlRecord (GML::ML::MLRecord &record);
	bool		GetRecord(GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask=0);
	bool		FreeMLRecord(GML::ML::MLRecord &record);
	bool		GetRecordLabel(double &label,UInt32 index);
	bool		GetFeatureName(GML::Utils::GString &str,UInt32 index);
	UInt32		GetFeatureCount();
	UInt32		GetRecordCount();
	UInt32		GetTotalRecordCount();
	
};
