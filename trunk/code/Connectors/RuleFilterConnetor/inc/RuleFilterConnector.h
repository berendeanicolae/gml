#ifndef __CONNECTOR_RuleFilterConnector__
#define __CONNECTOR_RuleFilterConnector__

#include "gmllib.h"

class RuleFilterConnector : public GML::ML::IConnector
{	
	enum
	{
		Method_RemoveIfAllFeaturesAreSet = 0,
		Method_KeepIfAllFeaturesAreSet,
		Method_RemoveIfOneFeatureIsSet,
		Method_KeepIfOneFeatureIsSet,
	};
	GML::Utils::Indexes				Indexes;
	GML::Utils::GTFVector<bool>		KeepRecords;
	GML::Utils::GString				RulesFile;
	UInt32							Method;
	GML::Utils::BitSet				FList;
	
	bool					FeatureNameToIndex(GML::Utils::GString &name,UInt32 &index);
	bool					LoadFeaturesList();
	bool					OnInitConnectionToConnector();
	
	bool					Check_AllFeaturesAreSet(GML::ML::MLRecord &rec);
	bool					Check_OneFeatureIsSet(GML::ML::MLRecord &rec);
	bool 					CheckRules(GML::ML::ConnectorThreadData &thData);
	void 					OnRunThreadCommand(GML::ML::ConnectorThreadData &thData,UInt32 threadCommand);
public:
	RuleFilterConnector();
	~RuleFilterConnector();

		
	bool					GetRecordLabel( double &label,UInt32 index );
	bool					GetRecord( GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask=0 );
	bool					GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index);
		
	bool					CreateMlRecord( GML::ML::MLRecord &record );
	bool					FreeMLRecord( GML::ML::MLRecord &record );
	
	bool					Close();
};

#endif
