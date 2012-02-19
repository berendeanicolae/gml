#include "ProbVectorMachine.h"
#include "KernelFunctionDBG.h"

#include "KernelWrapper.h"

ProbVectorMachine::ProbVectorMachine()
{
	ObjectName = "ProbVectorMachine";

	//Add extra commands here
	SetPropertyMetaData("Command","!!LIST:None=0,TestMachineSpeed,PreCompute!!");
	
	// kernel choice related variabiles	
	LinkPropertyToUInt32("KernelType",VarKernelType,KerFuncType::KERPOLY,"!!LIST:Poly=0,Scalar,Rbf,PolyParam,ScalarParam,RbfParam!!");
	LinkPropertyToInt32("KernelParamInt",VarKernelParamInt,0,"The Integer parameter of the kernel function");
	LinkPropertyToDouble("KernelParamDouble",VarKernelParamDouble,0,"The Double parameter of the kernel");
	LinkPropertyToString("FeatureWeightFile",VarFeatureWeightFile, "feature-weight.txt", "File name to hold feature weights; comma separated values");	

	// precompute related variables 	
	LinkPropertyToUInt32("PreCacheFileSize",VarPreCacheFileSize,1024,"The PreCache file size in MB");
	LinkPropertyToUInt32("PreCacheBatchStart",VarPreCacheBatchStart,0,"The PreCache start batch index");
	LinkPropertyToUInt32("PreCacheBatchNumber",VarPreCacheBatchCount,0,"The PreCache number of bathes to compute here");
	LinkPropertyToString("PreCacheFilePrefix",VarPreCacheFilePrefix, "precache", "File pattern where precomputed data to be saved; ex: pre-cache-data.000, pre-cache-data.001");	
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
		case THREAD_COMMAND_PRECOMPUTE_BATCH:
			InstPreCache.ThreadPrecomputeBatch(thData);
			return;
		default:
			ERRORMSG("could not find thread comment");
			return;

		// add extra thread command processes here
	};
}
bool ProbVectorMachine::OnInitThreadData(GML::Algorithm::MLThreadData &thData)
{
	thData.Context = NULL;
	return true;
}

void ProbVectorMachine::OnExecute()
{
	switch (Command)
	{
		case COMMAND_NONE:
			INFOMSG("Nothing to do, select another command");
			break;
        case COMNAND_TEST_MACHINE_SPEED:
            INFOMSG("Computing machine speed");            
            TestMachineSpeed();
            break;
		case COMMAND_PRECOMPUTE:
			INFOMSG("Precomputing Kernel Values");
			PreComputeKernelValues();
			break;
		case COMMAND_TEMP_KERNEL_FNCTS:
            INFOMSG("Andrei testing");            
			ker_f_dbg::exec_kernel_func_dbg(con, notif);
            break;
		default:
			notif->Error("[%s] -> Unknown command ID: %d",ObjectName,Command);
			break;
	}	
}

bool ProbVectorMachine::TestMachineSpeed()
{
    INFOMSG("Testing computational power with all threads for 1GB of dist");    
    //DBGSTOP_NULLCHECKMSG(NULL, "this is a NULL test");

    int nrOuts = UNGIGA/sizeof(pvm_float);
    int nrRec  = con->GetRecordCount();
    int nrLines = nrOuts/nrRec;

    // initialize thread data 
    MapTsc.nrLines = nrLines;
    MapTsc.nrRecords = nrRec;

    // allocated the necesary memory
    pvm_float* Buffer = (pvm_float*) malloc(UNGIGA);
    NULLCHECKMSG(Buffer, "Could not allocate enough memory");
    MapTsc.buffer = Buffer;  

    // the actual computing
    ExecuteParalelCommand(THREAD_COMMAND_TEST_PROC_SPEED);

	// write the data to the disk
	GML::Utils::File file;
	CHECKMSG(file.Create("1gb-memory.dat"), "could not create output file");

	int iter = 0;
	unsigned char*  tmpBuf = (unsigned char*)Buffer;
	
	notif->StartProcent("[%s] -> Writing to disk 1GB of dist... ",ObjectName);	
	while (iter < 1024) {
		DBG_CHECKMSG(file.Write(tmpBuf, UNMEGA), "could not write to file");		
		tmpBuf  += UNMEGA;
		iter++;
		notif->SetProcent(iter, 1024);
	}
	notif->EndProcent();
	file.Close();

	// read the date from the disk back	
	tmpBuf = (unsigned char*)Buffer;
	CHECKMSG(file.OpenRead("1gb-memory.dat"), "could not open file for reading");

	notif->StartProcent("[%s] -> Reading disk 1GB of dist... ",ObjectName);	
	iter = 0;
	while (iter < 1024) {
		DBG_CHECKMSG(file.Read(tmpBuf, UNMEGA),"could not read from file");
		tmpBuf  += UNMEGA;
		iter++;
		notif->SetProcent(iter, 1024);
	}
	notif->EndProcent();

    return true;
}

bool ProbVectorMachine::ThreadTestCompSpeed(GML::Algorithm::MLThreadData & thData)
{
    int threadId = thData.ThreadID;
    int ftrCount = con->GetFeatureCount();
    GML::ML::MLRecord first, second;    
    pvm_float sum = 0;

	CHECKMSG(con->CreateMlRecord(first), "Unable to create First record");
	CHECKMSG(con->CreateMlRecord(second), "Unable to create Second record");

	if (threadId==0) notif->StartProcent("[%s] -> Computing 1GB of distances ... ",ObjectName);

    // the first for is for lines, every thread takes one line at a time
    for (int i=threadId;i<MapTsc.nrLines;i+=threadsCount) {
        // get the first recod data
        con->GetRecord(first, i);

        // the second for computed a single line
        for (int j=0;j<MapTsc.nrRecords;j++) {
            // get the second record data    
            con->GetRecord(second, j);

            // the computing of the distantce
            sum = 0;
            for (int k=0;k<ftrCount;k++) 
                sum += (pvm_float)((first.Features[k]-second.Features[k])*(first.Features[k]-second.Features[k]));

            // put the output in the buffer
            MapTsc.buffer[i*MapTsc.nrRecords+j] = sum;
        }
		if (threadId==0) notif->SetProcent(i,MapTsc.nrLines);
    }
	if (threadId==0) notif->EndProcent();

    return true;
}

bool ProbVectorMachine::PreComputeKernelValues()
{
	PreCache::InheritData id;
	
	id.con = this->con;
	id.notif = this->notif;
	
	id.VarKernelType = this->VarKernelType;
	id.VarKernelParamInt = this->VarKernelParamInt;
	id.VarKernelParamDouble = this->VarKernelParamDouble;

	id.VarPreCacheBatchCount = this->VarPreCacheBatchCount;
	id.VarPreCacheBatchStart = this->VarPreCacheBatchStart;
	id.VarPreCacheFileSize = this->VarPreCacheFileSize;
	id.VarPreCacheFilePrefix.Add(this->VarPreCacheFilePrefix);
	
	// set PreCache needed information
	InstPreCache.SetInheritData(id);
	InstPreCache.SetParentAlg(this);

	CHECKMSG(InstPreCache.PreCompute(),"failed to compute precache");
	return true;
}