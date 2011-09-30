#ifndef __HashWriter__
#define __HashWriter__

#include "gmllib.h"

struct HashWriterThreadData
{
	// add thread specific data here
};

struct FeatInfo
{
	GML::DB::RecordHash		fHash;
	GML::DB::RecordHash		rHash;
	UInt32					Index;
	double					Label;
};


class HashWriter: public GML::Algorithm::IMLAlgorithm
{
	enum {
		COMMAND_NONE = 0,
		COMMAND_SHOW_NEGATIVE,
		COMMAND_SHOW_POSITIVE,
		COMMAND_SHOW_ALL_IN_1,
		COMMAND_SHOW_ALL_IN_2,
		COMMAND_SAVE_ALL,
		COMMAND_FEAT_COMB_GROUPS,
		//Add extra commands here
	};
	enum {
		THREAD_COMMAND_NONE = 0,
		//Add extra thread commands here
	};
	GML::ML::MLRecord		MainRecord;
	GML::Utils::GString		OutputFileName;
	GML::Utils::GString		OutputFileNamePositive;
	GML::Utils::GString		OutputFileNameNegative;

	GML::Utils::GTFVector<FeatInfo>		FList;
	bool								SaveFeaturesNames;
	bool								SaveFeaturesHash;

	void				OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool				OnInitThreadData(GML::Algorithm::MLThreadData &thData);
	bool 				SaveHashes(UInt32 command);
	bool 				SaveAll();
	bool				SaveHashGroupsByFeatComb();
	bool				LoadRecords(GML::Algorithm::MLThreadData &thData);
	bool				GetFeatures(GML::Utils::GString	&features, UInt32 recIndex);
	void				PrintFlist();
public:
	HashWriter();

	bool				Init();
	void				OnExecute();
};

#endif

