#ifndef __HashWriter__
#define __HashWriter__

#include "gmllib.h"

struct HashWriterThreadData
{
	// add thread specific data here
};

class HashWriter: public GML::Algorithm::IMLAlgorithm
{
	enum {
		COMMAND_NONE = 0,
		COMMAND_SHOW_NEGATIVE,
		COMMAND_SHOW_POSITIVE,
		COMMAND_SHOW_ALL_IN_1,
		COMMAND_SHOW_ALL_IN_2
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

	void				OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool				OnInitThreadData(GML::Algorithm::MLThreadData &thData);
	bool 				SaveHashes(UInt32 command);
public:
	HashWriter();

	bool				Init();
	void				OnExecute();
};

#endif

