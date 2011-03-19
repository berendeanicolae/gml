#include "gmllib.h"

class MapConnector: public GML::ML::IConnector
{
	enum
	{
		UseAND = 0,
		UseOR,
		UseXOR,
		UseAnd_Or,

	};

	UInt32	mapMethod;
	UInt32	featuresCount;	

public:
	MapConnector();

	bool		OnInit();
	bool		Close();
	bool		SetRecordInterval(UInt32 start, UInt32 end);
	bool		CreateMlRecord (GML::ML::MLRecord &record);
	bool		GetRecord(GML::ML::MLRecord &record,UInt32 index);
	bool		FreeMLRecord(GML::ML::MLRecord &record);
	bool		GetRecordLabel(double &label,UInt32 index);
	UInt32		GetFeatureCount();
	UInt32		GetRecordCount();
	UInt32		GetTotalRecordCount();
	
};
