#ifndef __LinearError__
#define __LinearError__

#include "gmllib.h"

struct LinearErrorThreadData
{
	double	*delta;	
	double	deltaBias;
	UInt32	countErrors;
	UInt32	countPozitive;
	UInt32	countNegative;
};

class LinearError: public GML::Algorithm::IMLAlgorithm
{
	enum {
		COMMAND_NONE = 0,
		COMMAND_COMPUTE,
		//Add extra commands here
	};
	enum {
		THREAD_COMMAND_NONE = 0,
		THREAD_COMMAND_TRAIN,
		//Add extra thread commands here
	};
	enum {
		SAVE_DATA_WhenAlgorithmEnds = 0,
		SAVE_DATA_AfterEachIteration,
	};
	enum {
		FILTER_NONE = 0,
		FILTER_VALUES,
		FILTER_PERCENTAGE,
	};
	GML::ML::MLRecord		MainRecord;
	GML::Utils::GString		ResultFileName;
	double*					Weight;
	double					Bias;
	double					LearningRate;
	double					Min,Max;
	bool					UseBias;
	UInt32*					RecordErrors;
	UInt32					MaxIterations;
	UInt32					SaveData;
	UInt32					Filter;
	

	void				OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool				OnInitThreadData(GML::Algorithm::MLThreadData &thData);
	bool				TrainIteration(GML::Algorithm::MLThreadData &thData);
	
	void				ComputeErrors();
	bool				SaveResultTable(char *fileName);
public:
	LinearError();

	bool				Init();
	void				OnExecute();
};

#endif

