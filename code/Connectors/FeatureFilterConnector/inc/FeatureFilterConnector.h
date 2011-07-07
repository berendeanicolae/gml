#include "gmllib.h"

using namespace GML;
using namespace GML::DB;
using namespace GML::ML;
using namespace GML::Utils;




class FeatureFilterConnector : public IConnector
{
protected:

	enum {
		FILTER_REMOVE_FEATURE = 0,
		FILTER_KEEP_FEATURE
	};
private:
	GML::Utils::GTFVector<GML::DB::RecordHash>	HashList;
	GML::Utils::Indexes							Indexes;

	//properties
	UInt32										FilterMethod;
	GML::Utils::GString							FeatureFileName;


	bool					LoadTextHashFile();
	
	bool					OnInitConnectionToConnector();
public:
	FeatureFilterConnector();
	~FeatureFilterConnector();

	bool					GetRecordLabel( double &label,UInt32 index );
	bool					GetRecord( MLRecord &record,UInt32 index,UInt32 recordMask=0 );
	bool					GetFeatureName(GML::Utils::GString &str,UInt32 index);
	bool					CreateMlRecord( MLRecord &record );
	bool					FreeMLRecord( MLRecord &record );
	bool					Close();
};

