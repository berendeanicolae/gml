#ifndef __ProbVectorMachine__
#define __ProbVectorMachine__

#include "gmllib.h"

#include "PVMDefinesInclude.h"

struct TdTestSpeedCompute
{
    int nrLines;
    int nrRecords;
    pvm_float * buffer;
};

class ProbVectorMachine: public GML::Algorithm::IMLAlgorithm
{
	enum {
		COMMAND_NONE = 0,
        COMNAND_TEST_MACHINE_SPEED,
        COMMAND_TEMP_KERNEL_FNCTS,
		//Add extra commands here
	};
	enum {
		THREAD_COMMAND_NONE = 0,
        THREAD_COMMAND_TEST_PROC_SPEED,
		//Add extra thread commands here
	};
	GML::ML::MLRecord		MainRecord;

	void				OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);
	bool				OnInitThreadData(GML::Algorithm::MLThreadData &thData);

    bool            TestMachineSpeed();

    // thread data part
    GML::Algorithm::MLThreadData* PvmThreadData;

public:
	ProbVectorMachine();

	bool			Init();
	void			OnExecute();    
    bool ThreadTestCompSpeed( GML::Algorithm::MLThreadData & thData );
};

#endif

