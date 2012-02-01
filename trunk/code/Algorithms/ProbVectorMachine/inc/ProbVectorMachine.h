#ifndef __ProbVectorMachine__
#define __ProbVectorMachine__

#include "gmllib.h"

struct ProbVectorMachineThreadData
{
	// add thread specific data here
};

class ProbVectorMachine: public GML::Algorithm::IMLAlgorithm
{
	enum {
		COMMAND_NONE = 0,
		//Add extra commands here
	};
	enum {
		THREAD_COMMAND_NONE = 0,
		//Add extra thread commands here
	};
	GML::ML::MLRecord		MainRecord;

	void				OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool				OnInitThreadData(GML::Algorithm::MLThreadData &thData);
public:
	ProbVectorMachine();

	bool				Init();
	void				OnExecute();
};

#endif

