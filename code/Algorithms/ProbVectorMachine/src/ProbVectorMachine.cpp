#include "ProbVectorMachine.h"
#include "KernelFunctionDBG.h"

ProbVectorMachine::ProbVectorMachine()
{
	ObjectName = "ProbVectorMachine";

	//Add extra commands here
	SetPropertyMetaData("Command","!!LIST:None=0,TestMachineSpeed,TempTestKernel!!");    
}
bool ProbVectorMachine::Init()
{
    CHECKMSG(InitConnections(), "Could not initialize connections");
    CHECKMSG(InitThreads(), "Could not initialize threads");
    CHECKMSG(con->CreateMlRecord(MainRecord), "Could not create main ML Record");	
	return true;
}
void ProbVectorMachine::OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand)
{
	switch (threadCommand)
	{
		case THREAD_COMMAND_NONE:
            return;			
        case THREAD_COMMAND_TEST_PROC_SPEED:
            ThreadTestCompSpeed(thData);            
			return;
		// add extra thread command processes here
	};
}
bool ProbVectorMachine::OnInitThreadData(GML::Algorithm::MLThreadData &thData)
{
	thData.Context = NULL;
    this->PvmThreadData = &thData;
	return true;
}

void ProbVectorMachine::OnExecute()
{
	switch (Command)
	{
		case COMMAND_NONE:
			INFOMSG("Nothing to do , select another command");
			break;
        case COMNAND_TEST_MACHINE_SPEED:
            INFOMSG("Computing machine speed");            
            TestMachineSpeed();
            break;
        case COMMAND_TEMP_KERNEL_FNCTS:
            INFOMSG("Andrei testing");            
			ker_f_dbg::exec_kernel_func_dbg(con);
            break;
		default:
			notif->Error("[%s] -> Unknown command ID: %d",ObjectName,Command);
			break;
	}	
}

bool ProbVectorMachine::TestMachineSpeed()
{
    INFOMSG("Testing computational power with all threads for 1Gb of results");
    return true;
    //DBGSTOP_NULLCHECKMSG(NULL, "this is a NULL test");

    int nrOuts = UNGIGA/sizeof(pvm_float);
    int nrRec  = con->GetRecordCount();
    int nrLines = nrOuts/nrRec;

    // initialize thread data 
    TdTestSpeedCompute ttsc;
    ttsc.nrLines = nrLines;
    ttsc.nrRecords = nrRec;

    // allocated the necesary memory
    pvm_float* Buffer = (pvm_float*) malloc(UNGIGA);
    NULLCHECKMSG(Buffer, "Could not allocate enough memory");
    ttsc.buffer = Buffer;  

    // link this structure to the threads structure
    this->PvmThreadData->Context = (void*) &ttsc;

    // the actual computing
    //notif->StartProcent()
    ExecuteParalelCommand(THREAD_COMMAND_TEST_PROC_SPEED);

    return true;
}

bool ProbVectorMachine::ThreadTestCompSpeed( GML::Algorithm::MLThreadData & thData )
{
    int ThreadID = thData.ThreadID;
    int ftrCount = con->GetFeatureCount();
    GML::ML::MLRecord first, second;
    TdTestSpeedCompute* ttsc = (TdTestSpeedCompute*)thData.Context;
    pvm_float sum = 0;

    // the first for is for lines, every thread takes one line at a time
    for (int i=ThreadID;i<ttsc->nrLines;i+=threadsCount) {
        // get the first recod data
        con->GetRecord(first, i);

        // the second for computed a single line
        for (int j=0;j<ttsc->nrRecords;j++) {
            // get the second record data    
            con->GetRecord(second, j);

            // the computing of the distantce
            sum = 0;
            for (int k=0;k<ftrCount;k++) 
                sum += (pvm_float)((first.Features[k]-second.Features[k])*(first.Features[k]-second.Features[k]));

            // put the output in the buffer
            ttsc->buffer[i*ttsc->nrRecords+j] = sum;
        }
    }

    return true;
}
