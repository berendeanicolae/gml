#include "gmllib.h"

using namespace GML;
using namespace GML::DB;
using namespace GML::ML;
using namespace GML::Utils;


struct FeatureIndex
{
	GML::DB::RecordHash		Hash;
	UInt32					Index;
};

class FeatureReorderConnector : public IConnector
{
private:
	GML::Utils::GTFVector<FeatureIndex>			HashList;
	GML::Utils::Indexes							Indexes;

	//properties
	UInt32										FilterMethod;
	GML::Utils::GString							FeatureFileName;


	bool					LoadTextHashFile();	
	bool					OnInitConnectionToConnector();
public:
	FeatureReorderConnector();
	~FeatureReorderConnector();

	bool					GetRecordLabel( double &label,UInt32 index );
	bool					GetRecord( MLRecord &record,UInt32 index,UInt32 recordMask=0 );
	bool					GetFeatureName(GML::Utils::GString &str,UInt32 index);
	bool					CreateMlRecord( MLRecord &record );
	bool					FreeMLRecord( MLRecord &record );
	bool					Close();
};

