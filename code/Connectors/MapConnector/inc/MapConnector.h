#include "gmllib.h"

class MapConnector: public GML::ML::IConnector
{
	enum
	{
		UseAND = 0,
		UseOR,
		UseXOR,
		UseAnd_Or,
		UseMultiply,
		UseAddition,
		UseFeatAverage,
		Negate,
		Interval,

	};

	UInt32		mapMethod;
	double		startMapInterval,endMapInterval;
	GML::Utils::GTFVector<UInt32>	NameIndex;

	bool		CreateNameIndexes();
	bool		OnInitConnectionToConnector();

public:
	MapConnector();

	bool		Close();
	
	bool		CreateMlRecord (GML::ML::MLRecord &record);
	bool		GetRecord(GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask=0);
	bool		GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index);
	
	bool		FreeMLRecord(GML::ML::MLRecord &record);
	bool		GetRecordLabel(double &label,UInt32 index);

	bool		GetFeatureName(GML::Utils::GString &str,UInt32 index);
};
