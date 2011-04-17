#include "gmllib.h"

using namespace GML;
using namespace GML::DB;
using namespace GML::ML;
using namespace GML::Utils;




class HashFilterConnector : public IConnector
{
protected:

	enum {
		FILTER_REMOVE_HASH = 0,
		FILTER_KEEP_HASH
	};
private:
	UInt32										FeatureCount,RecordCount;
	GML::Utils::GTFVector<GML::DB::RecordHash>	HashList;
	GML::Utils::Indexes							Indexes;

	//properties
	UInt32										HashFileType;
	UInt32										FilterMethod;
	GML::Utils::GString							HashFileName;


	bool					LoadBinaryHashFile();

	
	bool					OnInitConnectionToConnector();
public:
	HashFilterConnector();
	~HashFilterConnector();

	UInt32					GetRecordCount();
	bool					GetRecordLabel( double &label,UInt32 index );
	UInt32					GetFeatureCount();	
	bool					GetRecord( MLRecord &record,UInt32 index,UInt32 recordMask=0 );
	bool					GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index);
	bool					CreateMlRecord( MLRecord &record );
	bool					SetRecordInterval( UInt32 start, UInt32 end );
	bool					FreeMLRecord( MLRecord &record );
	bool					Close();
	UInt32					GetTotalRecordCount();	
	bool					OnInitPercentage();
	bool					GetFeatureName(GML::Utils::GString &str,UInt32 index);
};

