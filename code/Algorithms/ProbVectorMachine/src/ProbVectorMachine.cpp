#include "ProbVectorMachine.h"
#include "KernelFunctionDBG.h"

#include "KernelWrapper.h"

ProbVectorMachine::ProbVectorMachine()
{
	ObjectName = "ProbVectorMachine";

	//Add extra commands here
	SetPropertyMetaData("Command","!!LIST:None=0,DebugTest,PreCompGramMatrix,MergeKprimeFiles,PreCompEqNorm,"
						"InitStateVars,BlockTraining,LastBlockTraining,GatherBlockStates,Clasify!!");
	
	// kernel choice related variables	
	LinkPropertyToUInt32("KernelType",varKernelType,KERPOLY,"!!LIST:Poly=0,Scalar,Rbf,PolyParam,ScalarParam,RbfParam!!");
	LinkPropertyToInt32("KernelParamInt",varKernelParamInt,0,"The Integer parameter of the kernel function");
	LinkPropertyToDouble("KernelParamDouble",varKernelParamDouble,0,"The Double parameter of the kernel");
	LinkPropertyToString("FeatureWeightFile",varFeatureWeightFile, "feature-weight.txt", "File name to hold feature weights; comma separated values");	

	// pre-compute related variables 	
	LinkPropertyToUInt32("BlockFileSize",varBlockFileSize,1024,"The PreCache file size in MB");
	LinkPropertyToUInt32("BlockStart",varBlockStart,0,"The start block index");
	LinkPropertyToUInt32("BlockCount",varBlockCount,0,"The number of blocks to compute here");
	LinkPropertyToString("BlockFilePrefix",varBlockFilePrefix, "pre-cache", "File pattern where precomputed data to be saved; ex: pre-cache-data.000, pre-cache-data.001");

	// algorithm related variables	
	LinkPropertyToDouble("Lambda",varLambda,1.0,"The Lambda parameter for this iteration");
	LinkPropertyToDouble("T",varT,1.0,"The T algorithm parameter; used for binary search");
	LinkPropertyToUInt32("WindowSize",varWindowSize,10,"The Windows size the block solver works with");
	LinkPropertyToUInt32("IterationNo",varIterNr,0,"The current iteration number");

	LinkPropertyToUInt32("NrUpdatesPerNormBlock",varNrUpdatesPerNormBlock,1,"Number of updates to be applied for a normal block");
	LinkPropertyToUInt32("NrUpdatesPerLastBlock",vadNrUpdatesPerLastBlock,1,"Numnber of updates to be applied for the last block");

	// final classification related vars
	LinkPropertyToString("ModelFile",varModelFile, "model-file.dat", "File with the algorithm model to be used for classification");
	LinkPropertyToString("ConnectorTest",varConectorTest,"","Test Connector string");

	conectorTest = NULL;
}
bool ProbVectorMachine::Init()
{
    CHECKMSG(InitConnections(), "Could not initialize connections");
	CHECKMSG(InitExtraConnections(), "Could not initialize the extra test connector");
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
		case THREAD_COMMAND_PRECOMPUTE_BLOCK:
			InstPreCache.ThreadPrecomputeBlock(thData);
			return;
		case THREAD_COMMAND_WINDOW_UPDATE:
			PerformWindowUpdate(thData);
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
        case COMMAND_DEBUG_TESTS:
            INFOMSG("Computing machine speed");            
            PreCacheCall(Command);
            break;
		case COMMAND_PRECOMP_GRAM:
			INFOMSG("Precomputing Kernel Values");
			PreCacheCall(Command);
			break;
		case COMMAND_MERGE_KPRIME:
			INFOMSG("Merging Kprime files");
			PreCacheCall(Command);
			break;
		case COMMAND_PRECOMP_NORM:
			INFOMSG("Precomputing the norm for each equation");
			PreCacheCall(Command);
			break;
		case COMMAND_BLOCK_TRAINING:
			INFOMSG("Starting the training session");
			IterateBlockTraining();
			break;
		case COMMAND_INIT_STATE_VARS:
			INFOMSG("Dumping default state variables to disk");
			DumpDefaultStateVariables();
			break;
		case COMMAND_LAST_BLOCK_TRAINING:
			INFOMSG("Starting the training for the last block");
			LastBlockTraining();
			break;
		case COMMAND_GATHER_BLOCK_STATES:
			INFOMSG("Starting to average the states of every block");
			GatherBlockStates();
			break;
		case COMMAND_CLASSIFY:
			INFOMSG("Starting classification");
			ClasifyDataset();
			break;

		default:
			notif->Error("[%s] -> Unknown command ID: %d",ObjectName,Command);
			break;
	}	
}

bool ProbVectorMachine::TestMachineSpeed()
{
    INFOMSG("Testing computational power with all threads for 1GB of dist");    
    //NULLCHECKMSG(NULL, "this is a NULL test");

    int nrOuts = UNGIGA/sizeof(pvm_float);
    int nrRec  = con->GetRecordCount();
    int nrLines = nrOuts/nrRec;

    // initialize thread data 
    mapTsc.nrLines = nrLines;
    mapTsc.nrRecords = nrRec;

    // allocated the necessary memory
    pvm_float* Buffer = (pvm_float*) malloc(UNGIGA);
    NULLCHECKMSG(Buffer, "Could not allocate enough memory");
    mapTsc.buffer = Buffer;  

    // the actual computing
    ExecuteParalelCommand(THREAD_COMMAND_TEST_PROC_SPEED);

	// write the data to the disk
	GML::Utils::File file;
	CHECKMSG(file.Create("1gb-memory.dat"), "could not create output file");

	int iter = 0;
	unsigned char*  tmpBuf = (unsigned char*)Buffer;
	
	notif->StartProcent("[%s] -> Writing to disk 1GB of dist... ",ObjectName);	
	while (iter < 1024) {
		CHECKMSG(file.Write(tmpBuf, UNMEGA), "could not write to file");		
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
		CHECKMSG(file.Read(tmpBuf, UNMEGA),"could not read from file");
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
    for (int i=threadId;i<mapTsc.nrLines;i+=threadsCount) {
        // get the first record data
        con->GetRecord(first, i);

        // the second for computed a single line
        for (int j=0;j<mapTsc.nrRecords;j++) {
            // get the second record data    
            con->GetRecord(second, j);

            // the computing of the distance
            sum = 0;
            for (int k=0;k<ftrCount;k++) 
                sum += (pvm_float)((first.Features[k]-second.Features[k])*(first.Features[k]-second.Features[k]));

            // put the output in the buffer
            mapTsc.buffer[i*mapTsc.nrRecords+j] = sum;
        }
		if (threadId==0) notif->SetProcent(i,mapTsc.nrLines);
    }
	if (threadId==0) notif->EndProcent();

    return true;
}

void ProbVectorMachine::PreCacheInstanceInit()
{
	PreCache::InheritData id;

	id.con = this->con;
	id.notif = this->notif;

	id.varKernelType = this->varKernelType;
	id.varKernelParamInt = this->varKernelParamInt;
	id.varKernelParamDouble = this->varKernelParamDouble;

	id.varBlockCount = this->varBlockCount;
	id.varBlockStart = this->varBlockStart;
	id.varBlockFileSize = this->varBlockFileSize;

	id.varBlockFilePrefix.Truncate(0);
	id.varBlockFilePrefix.Add(this->varBlockFilePrefix);

	// set PreCache needed information
	InstPreCache.SetInheritData(id);
	InstPreCache.SetParentAlg(this);
}

bool ProbVectorMachine::PreCacheCall(UInt32 cmd)
{
	
	PreCacheInstanceInit();

	switch (cmd) {
	case COMMAND_MERGE_KPRIME:
		CHECKMSG(InstPreCache.MergeKPrimeFiles(),"failed to merge kprime files");
		break;
	case COMMAND_DEBUG_TESTS:
		CHECKMSG(InstPreCache.AtInitLoading(),"failed to execute debug tests");
		break;
	case COMMAND_PRECOMP_GRAM:
		CHECKMSG(InstPreCache.PreComputeGram(),"failed to compute gram matrix");
		break;
	case COMMAND_PRECOMP_NORM:
		CHECKMSG(InstPreCache.PreComputeNorm(),"failed to precompute the form for each equation");
		break;
	default:
		CHECKMSG(false, "nothing to compute here, returning");
	}

	return true;
}

bool ProbVectorMachine::IterateBlockTraining()
{
	/*
	 Read initial state from the disk; this state is available through all the blocks 
	 that are being processed on this machine
	*/

	UInt32 nrRec = con->GetRecordCount();
	UInt32 vectSz = sizeof(pvm_float)*nrRec;
	UInt64 read, written;

	PreCache::BlockLoadHandle *handle;
	PreCache::KPrimePair	  *kprime;
	
	GML::Utils::File				fileObj;
	GML::Utils::GString				fileName;
	PreCache::PreCacheFileHeader	kpHeader;
	StateFileHeader					stateHeader;

	pvm_float *alpha, *sigma;
	
	alpha = (pvm_float*) malloc(vectSz);
	NULLCHECKMSG(alpha, "could not alloc memory for alphaOrig");

	sigma = (pvm_float*) malloc(vectSz);
	NULLCHECKMSG(sigma, "could not alloc memory for sigmaPOrig");

	// initialize state variables with values read from disk or init here
	fileName.Truncate(0);
	fileName.AddFormated("%s.state.iter.%03d.block.all", varBlockFilePrefix.GetText(), varIterNr-1);

	// read state variables from disk
	CHECKMSG(fileObj.OpenRead(fileName),"could not open file for reading: %s",fileName.GetText());
		
	CHECKMSG(fileObj.Read(&stateHeader, sizeof(StateFileHeader), &read), "could not read from state file");
	CHECKMSG(read==sizeof(StateFileHeader), "could not read enough from state file");

	CHECKMSG(fileObj.Read(alpha, vectSz, &read), "could not read from state file");
	CHECKMSG(read==vectSz, "could not read enough from state file");

	CHECKMSG(fileObj.Read(sigma, vectSz, &read), "could not read from state file");
	CHECKMSG(read==vectSz, "could not read enough from state file");
	fileObj.Close();
	
	// read kprime file
	
	fileName.Truncate(0);
	fileName.Add(varBlockFilePrefix);
	fileName.Add(".kprime.all");

	CHECKMSG(fileObj.OpenRead(fileName),"could not open file for reading: %s",fileName.GetText());
	CHECKMSG(fileObj.Read(&kpHeader, sizeof(PreCache::PreCacheFileHeader), &read), "could not read from file");
	CHECKMSG(sizeof(PreCache::PreCacheFileHeader)==read,"could not read enough from file");
	CHECKMSG(strcmp(kpHeader.Magic,KPRIME_FILE_HEADER_MAGIC)==0, "could not verify kprime header magic");
	CHECKMSG(sizeof(PreCache::KPrimePair)*nrRec==kpHeader.BlockSize, "kprime block size does not have the correct size");

	// alloc memory for kprime buffer
	kprime = (PreCache::KPrimePair*) malloc((size_t)kpHeader.BlockSize);
	NULLCHECKMSG(kprime, "could not alloc memory for sigmaPOrig");
					
	CHECKMSG(fileObj.Read(kprime, kpHeader.BlockSize, &read), "could not read from file");
	CHECKMSG(kpHeader.BlockSize==read,"could not read enough from file");
	fileObj.Close();

	// alloc memory for local state variables
	wu.ALPH = (pvm_float*) malloc(vectSz);
	NULLCHECKMSG(wu.ALPH, "could not alloc memory for alpha");

	wu.SIGM = (pvm_float*) malloc(vectSz);
	NULLCHECKMSG(wu.SIGM, "could not alloc memory for sigmaPlus");

	// copy the contents of the state variables read from disk
	memcpy(wu.ALPH, alpha, vectSz);
	memcpy(wu.SIGM, sigma, vectSz);	

	// alloc memory for window update structure
	wu.uALPH  = (pvm_float*) malloc(nrRec*varWindowSize*sizeof(pvm_float));	
	wu.uSIGM  = (pvm_float*) malloc(varWindowSize*sizeof(pvm_float));
	wu.san    = (pvm_float*) malloc(varWindowSize*sizeof(pvm_float));
	wu.pn     = (pvm_float*) malloc(varWindowSize*sizeof(pvm_float));
	NULLCHECKMSG(wu.uALPH && wu.uSIGM && wu.san && wu.pn, "could not alloc enough memory");

	// start processing blocks

	PreCacheInstanceInit();
	InstPreCache.AtInitLoading();

	UInt32 blkIdx;
	// signal the first load because the for will start by waiting
	InstPreCache.AtSignalStartLoading(varBlockStart);

	for (blkIdx=varBlockStart + 1; blkIdx<varBlockStart+varBlockCount; blkIdx++)
	{
		handle = InstPreCache.AtWaitForCompletion();						
		NULLCHECKMSG(handle,"could not load block file:%d exiting", blkIdx-1);		
		handle->KPRM = kprime;

		CHECKMSG(InstPreCache.AtSignalStartLoading(blkIdx),"could not signal loading of block:%d ", blkIdx);
		CHECKMSG(PerfomBlockTraining(blkIdx, handle), "error performing training on block: %d", blkIdx - 1);

		// dump the block state variables to disk	
		fileName.Truncate(0);	
		fileName.AddFormated("%s.state.iter.%03d.block.%03d", varBlockFilePrefix.GetText(), varIterNr, blkIdx - 1);

		stateHeader.blkNr = blkIdx - 1;
		stateHeader.recStart = handle->recStart;
		stateHeader.recCount = handle->recCount;
		stateHeader.totalRecCount = nrRec;

		CHECKMSG(fileObj.Create(fileName), "could not create file: %s", fileName.GetText());

		// write header
		CHECKMSG(fileObj.Write(&stateHeader,sizeof(StateFileHeader),&written),"could not write to block state file");
		CHECKMSG(written==sizeof(StateFileHeader),"could not write enough to the block state file");

		// write alphas
		CHECKMSG(fileObj.Write(wu.ALPH,vectSz,&written),"could not write to block state file");
		CHECKMSG(written==vectSz,"could not write enough to the block state file");

		// write signmas
		CHECKMSG(fileObj.Write(wu.SIGM,vectSz,&written),"could not write to block state file");
		CHECKMSG(written==vectSz,"could not write enough to the block state file");

		// write block score for stop condition
		CHECKMSG(fileObj.Write(&wu.score,sizeof(pvm_float),&written),"could not write to block state file");
		CHECKMSG(written==sizeof(pvm_float),"could not write enough to the block state file");

		fileObj.Close();
	}

	{
		//the last block must finish processing, as it was only signaled for loading
		//carefull : the blockIdx is now equal to varBlockStart+varBlockCount and this must not be changed
		handle = InstPreCache.AtWaitForCompletion();						
		NULLCHECKMSG(handle,"could not load block file:%d exiting", blkIdx-1);		
		handle->KPRM = kprime;

		CHECKMSG(PerfomBlockTraining(blkIdx, handle), "error performing training on block: %d", blkIdx - 1);

		// dump the block state variables to disk	
		fileName.Truncate(0);	
		fileName.AddFormated("%s.state.iter.%03d.block.%03d", varBlockFilePrefix.GetText(), varIterNr, blkIdx - 1);

		stateHeader.blkNr = blkIdx - 1;
		stateHeader.recStart = handle->recStart;
		stateHeader.recCount = handle->recCount;
		stateHeader.totalRecCount = nrRec;

		CHECKMSG(fileObj.Create(fileName), "could not create file: %s", fileName.GetText());

		// write header
		CHECKMSG(fileObj.Write(&stateHeader,sizeof(StateFileHeader),&written),"could not write to block state file");
		CHECKMSG(written==sizeof(StateFileHeader),"could not write enough to the block state file");

		// write alphas
		CHECKMSG(fileObj.Write(wu.ALPH,vectSz,&written),"could not write to block state file");
		CHECKMSG(written==vectSz,"could not write enough to the block state file");

		// write signmas
		CHECKMSG(fileObj.Write(wu.SIGM,vectSz,&written),"could not write to block state file");
		CHECKMSG(written==vectSz,"could not write enough to the block state file");

		// write block score for stop condition
		CHECKMSG(fileObj.Write(&wu.score,sizeof(pvm_float),&written),"could not write to block state file");
		CHECKMSG(written==sizeof(pvm_float),"could not write enough to the block state file");

		fileObj.Close();
	}

	// free all temporary used memory buffers
	free(alpha);
	free(sigma);
	free(kprime);
	free(wu.ALPH);
	free(wu.SIGM);
	free(wu.uALPH);
	free(wu.uSIGM);
	free(wu.san);
	free(wu.pn);

	return true;
}

bool ProbVectorMachine::PerfomBlockTraining(UInt32 blkIdx, PreCache::BlockLoadHandle *handle)
{
	// algorithm state variables
	pvm_float scoreSum, update;
	UInt32	  i, w, k, updateNr;
	pvm_float maxWindowScore;
	pvm_float minScoreSum = 1e-10f;

	UInt32 nrRec = con->GetRecordCount();
	for (updateNr = 0; updateNr < varNrUpdatesPerNormBlock; updateNr++)
	{
		wu.score = 0;

		wu.bHandle = handle;
		for (w=0;w<handle->recCount;w+=varWindowSize)
		{
			wu.winStart = w;
			if (handle->recCount-w < varWindowSize) wu.winSize = handle->recCount-w;
			else wu.winSize = varWindowSize;

			// compute window update 		
			ExecuteParalelCommand(THREAD_COMMAND_WINDOW_UPDATE);

			// compute shares for every score
			scoreSum = 0;
			maxWindowScore = FLT_MIN;
			for (i=0;i<wu.winSize;i++) {
				scoreSum += wu.san[i];

				// search for max score per window
				if (maxWindowScore < wu.san[i]) {
					maxWindowScore = wu.san[i];
				}
			}
			wu.score += maxWindowScore;

			if (scoreSum < minScoreSum)
				scoreSum = minScoreSum;
			
			for (i=0;i<wu.winSize;i++)
				wu.pn[i] = wu.san[i]/scoreSum;
						
			// update alphas
			for (i=0;i<nrRec;i++) {			
				update = 0;
				for (k=0;k<wu.winSize;k++) 
					update = wu.pn[k] * wu.uALPH[nrRec*k + i];					
			
				update *= (pvm_float)varLambda;
				wu.ALPH[i] += update;
			}

			// update sigmas
			for (k=0;k<wu.winSize;k++)
				wu.SIGM[handle->recStart+w+k] += (pvm_float)varLambda * wu.pn[k] * wu.uSIGM[k];		

			//if (w==0) w-=varWindowSize;
		}
		
		if (wu.score < 1e-8f)
			break;
	}
	return true;
}

bool ProbVectorMachine::PerformWindowUpdate(GML::Algorithm::MLThreadData &thData)
{
	UInt32		recIdxGlob, recIdxBlock, i, uALPH_it;
	double		label;
	pvm_float	sj, ker, kpr, frac;

	//INFOTHDMSG("perform window update");
	
	UInt32 nrRec = con->GetRecordCount();
	wu.updateNeeded = false;
	
	for (UInt32 winIt=thData.ThreadID; winIt<wu.winSize; winIt+=threadsCount)
	{
		recIdxBlock = wu.winStart + winIt;
		recIdxGlob = wu.bHandle->recStart + recIdxBlock; 		
		
		CHECKMSG(con->GetRecordLabel(label, recIdxGlob),"could not get record label");				
		
		sj = 0;
		wu.san[winIt] = 0;

		if (label == 1)
		{		
			for (i = 0; i < nrRec; i++) 
			{
				ker = KerAt(recIdxBlock, i, wu.bHandle->KERN, nrRec);
				kpr = wu.bHandle->KPRM[i].pos;
				sj += wu.ALPH[i] * (ker - kpr);
			}

			if (wu.SIGM[recIdxGlob] - sj < 0) 
			{
				wu.updateNeeded = true;
				frac = (sj - wu.SIGM[recIdxGlob]) / (wu.bHandle->NORM[recIdxBlock]*wu.bHandle->NORM[recIdxBlock]);				
				for (i = 0, uALPH_it = winIt*nrRec; i < nrRec; i++, uALPH_it++) 
				{
					ker = KerAt(recIdxBlock, i, wu.bHandle->KERN, nrRec);				
					kpr = wu.bHandle->KPRM[i].pos;
					wu.uALPH[uALPH_it] = frac * (kpr-ker);
				}
				wu.uSIGM[winIt] = frac;
				wu.san[winIt]	= (sj - wu.SIGM[recIdxGlob])/wu.bHandle->NORM[recIdxBlock];			
			} else if (wu.SIGM[recIdxGlob] + sj < 0) 
			{
				wu.updateNeeded = true;
				frac = (-sj - wu.SIGM[recIdxGlob]) / (wu.bHandle->NORM[recIdxBlock]*wu.bHandle->NORM[recIdxBlock]);								
				for (i = 0, uALPH_it = winIt*nrRec; i < nrRec; i++, uALPH_it++) 
				{
					ker = KerAt(recIdxBlock, i, wu.bHandle->KERN, nrRec);
					kpr = wu.bHandle->KPRM[i].pos;
					wu.uALPH[uALPH_it] = frac * (ker-kpr);
				}
				wu.uSIGM[winIt] = frac;
				wu.san[winIt]	= (-sj - wu.SIGM[recIdxGlob])/wu.bHandle->NORM[recIdxBlock];			
			} 
		}
		else
		{
			for (i = 0; i < nrRec; i++) 
			{							
				ker = KerAt(recIdxBlock, i, wu.bHandle->KERN, nrRec);
				kpr = wu.bHandle->KPRM[i].neg;
				sj += wu.ALPH[i] * (ker - kpr);
			}

			if (wu.SIGM[recIdxGlob] - sj < 0) 
			{
				wu.updateNeeded = true;
				frac = (sj - wu.SIGM[recIdxGlob]) / (wu.bHandle->NORM[recIdxBlock]*wu.bHandle->NORM[recIdxBlock]);
				for (i = 0, uALPH_it = winIt*nrRec; i < nrRec; i++, uALPH_it++) 
				{
					ker = KerAt(recIdxBlock, i, wu.bHandle->KERN, nrRec);				
					kpr = wu.bHandle->KPRM[i].neg;
					wu.uALPH[uALPH_it] = frac * (kpr-ker);
				}
				wu.uSIGM[winIt] = frac;
				wu.san[winIt]	= (sj - wu.SIGM[recIdxGlob])/wu.bHandle->NORM[recIdxBlock];
			} else if (wu.SIGM[recIdxGlob] + sj < 0) 
			{
				wu.updateNeeded = true;
				frac = (-sj - wu.SIGM[recIdxGlob]) / (wu.bHandle->NORM[recIdxBlock]*wu.bHandle->NORM[recIdxBlock]);				
				for (i = 0, uALPH_it = winIt*nrRec; i < nrRec; i++, uALPH_it++) 
				{
					ker = KerAt(recIdxBlock, i, wu.bHandle->KERN, nrRec);
					kpr = wu.bHandle->KPRM[i].neg;
					wu.uALPH[uALPH_it] = frac * (ker-kpr);
				}
				wu.uSIGM[winIt] = frac;
				wu.san[winIt]	= (-sj - wu.SIGM[recIdxGlob])/wu.bHandle->NORM[recIdxBlock];
			} 
		}	
	}
	return true;
}

inline pvm_float ProbVectorMachine::KerAt(UInt32 line,UInt32 row, pvm_float* ker, UInt32 nrRec)
{
	if (line>row) { UInt32 aux = line; line = row; row=aux; }
	return ker[(line*nrRec - line*(line-1)/2) + row-line];
}

bool ProbVectorMachine::LastBlockTraining()
{
	UInt32 nrRec = con->GetRecordCount(), i, updateNr;
	UInt32 vectSz = sizeof(pvm_float)*nrRec;
	UInt64 read, written;
	pvm_float varTFloat = (pvm_float)varT;

	PreCache::KPrimePair	  *kprime;

	GML::Utils::File				fileObj;
	GML::Utils::GString				fileName;
	PreCache::PreCacheFileHeader	kpHeader;
	StateFileHeader					stateHeader;

	pvm_float norm0, norm1, norm2, norm3, term0, term1;
	pvm_float maxScore, temp_mul;

	pvm_float *alpha, *sigma, b;

	pvm_float s[4];
	UpdateStr u[4];

	alpha = (pvm_float*) malloc(vectSz);
	NULLCHECKMSG(alpha, "could not alloc memory for alphaOrig");

	sigma = (pvm_float*) malloc(vectSz);
	NULLCHECKMSG(sigma, "could not alloc memory for sigmaPOrig");

	// initialize state variables with values read from disk or init here
	fileName.Truncate(0);
	fileName.AddFormated("%s.state.iter.%03d.block.all", varBlockFilePrefix.GetText(), varIterNr-1);

	// read state variables from disk
	CHECKMSG(fileObj.OpenRead(fileName),"could not open file for reading: %s",fileName.GetText());

	CHECKMSG(fileObj.Read(&stateHeader, sizeof(StateFileHeader), &read), "could not read from state file");
	CHECKMSG(read==sizeof(StateFileHeader), "could not read enough from state file");

	CHECKMSG(fileObj.Read(alpha, vectSz, &read), "could not read from state file");
	CHECKMSG(read==vectSz, "could not read enough from state file");

	CHECKMSG(fileObj.Read(sigma, vectSz, &read), "could not read from state file");
	CHECKMSG(read==vectSz, "could not read enough from state file");

	CHECKMSG(fileObj.Read(&b, sizeof(pvm_float), &read), "could not read from state file");
	CHECKMSG(read==sizeof(pvm_float), "could not read enough from state file");

	fileObj.Close();

	// read kprime file

	fileName.Truncate(0);
	fileName.Add(varBlockFilePrefix);
	fileName.Add(".kprime.all");

	CHECKMSG(fileObj.OpenRead(fileName),"could not open file for reading: %s",fileName.GetText());
	CHECKMSG(fileObj.Read(&kpHeader, sizeof(PreCache::PreCacheFileHeader), &read), "could not read from file");
	CHECKMSG(sizeof(PreCache::PreCacheFileHeader)==read,"could not read enough from file");
	CHECKMSG(strcmp(kpHeader.Magic,KPRIME_FILE_HEADER_MAGIC)==0, "could not verify kprime header magic");
	CHECKMSG(sizeof(PreCache::KPrimePair)*nrRec==kpHeader.BlockSize, "kprime block size does not have the correct size");

	// alloc memory for kprime buffer
	kprime = (PreCache::KPrimePair*) malloc((size_t)kpHeader.BlockSize);
	NULLCHECKMSG(kprime, "could not alloc memory for sigmaPOrig");

	CHECKMSG(fileObj.Read(kprime, kpHeader.BlockSize, &read), "could not read from file");
	CHECKMSG(kpHeader.BlockSize==read,"could not read enough from file");
	fileObj.Close();


	norm2 = 1;
	norm3 = 1;
	for (i=0;i<nrRec;i++) {
		norm2 += kprime[i].pos*kprime[i].pos;
		norm3 += kprime[i].neg*kprime[i].neg;
	}

	int Splus, Sminus;
	double label;
	Splus = Sminus = 0;
	for (i=0;i<nrRec;i++) {
		CHECKMSG(con->GetRecordLabel(label, i), "could not get record:%d label", i);
		if (label==1) Splus++;
		else Sminus++;
	}
	
	norm0 = varTFloat * varTFloat  * norm2 + Splus / ((Splus-1)*(Splus-1));
	norm1 = varTFloat * varTFloat  * norm3 + Sminus/ ((Sminus-1)*(Sminus-1));

	norm0 = sqrt(norm0);
	norm1 = sqrt(norm1);
	norm2 = sqrt(norm2);
	norm3 = sqrt(norm3);

	memset(u, 0, sizeof(UpdateStr)*4);
	memset(s, 0, sizeof(pvm_float)*4);

	for (updateNr = 0; updateNr < vadNrUpdatesPerLastBlock; updateNr++)
	{	
		term0 = term1 = b;
		for (i=0;i<nrRec;i++) {
			term0 += alpha[i]*kprime[i].pos;
			term1 += alpha[i]*kprime[i].neg;
		}
		term1 = -term1;

		pvm_float sumSigmaPlus = 0;
		pvm_float sumSigmaMinus = 0;
		for (i=0;i<nrRec;i++) {
			CHECKMSG(con->GetRecordLabel(label, i), "could not get record:%d label", i);
			if (label==1)
				sumSigmaPlus += sigma[i];
			else
				sumSigmaMinus += sigma[i];
		}		

		// first equation

		temp_mul = (pvm_float)(Splus - 1);
		s[0] = varTFloat * term0 - sumSigmaPlus / temp_mul;
	
		if (s[0] < 0) {
			if (u[0].alpha == NULL)
			{
				u[0].alpha = (pvm_float*) malloc(nrRec*sizeof(pvm_float));
				CHECKMSG(u[0].alpha, "could not alloc memory");
			}

			u[0].infeas_eq = true;		

			u[0].sigmaVal = -(pvm_float)1.0 / temp_mul;
			u[0].b = varTFloat;
			u[0].score = -s[0]/norm0;
			u[0].firstMember = -s[0]/(norm0*norm0);
		
			temp_mul = varTFloat * u[0].firstMember;
			for (i=0;i<nrRec;i++)
			   u[0].alpha[i] = temp_mul * kprime[i].pos;
		}

		// second equation

		temp_mul = (pvm_float)(Sminus-1);
		s[1] = varTFloat * term1 - sumSigmaMinus / temp_mul;
		if (s[1]<0) {
			if (u[1].alpha == NULL)
			{
				u[1].alpha = (pvm_float*) malloc(nrRec*sizeof(pvm_float));
				CHECKMSG(u[1].alpha, "could not alloc memory");
			}
			u[1].infeas_eq = true;

			u[1].sigmaVal = -(pvm_float)1.0 / temp_mul;
			u[1].b = -varTFloat;
			u[1].score = -s[1]/norm1;
			u[1].firstMember = -s[1]/(norm1*norm1);

			temp_mul = varTFloat * u[1].firstMember;
			for (i=0;i<nrRec;i++)
				u[1].alpha[i] = temp_mul * (-kprime[i].neg);
		}

		// third equation
		if (term0<0) {
			if (u[2].alpha == NULL)
			{
				u[2].alpha = (pvm_float*) malloc(nrRec*sizeof(pvm_float));
				CHECKMSG(u[2].alpha, "could not alloc memory");
			}

			u[2].infeas_eq = true;

			u[2].sigmaVal = 0;
			u[2].b = 1;
			u[2].score = (1-term0)/norm2;
			u[2].firstMember = (1-term0)/(norm2*norm2);

			for (i=0;i<nrRec;i++)
				u[2].alpha[i] = u[2].firstMember * (kprime[i].pos);
		}

		// forth equation
		if (term1<0) {
			if (u[3].alpha == NULL)
			{
				u[3].alpha = (pvm_float*) malloc(nrRec*sizeof(pvm_float));
				CHECKMSG(u[3].alpha, "could not alloc memory");
			}
			u[3].infeas_eq = true;

			u[3].sigmaVal = 0;
			u[3].b = -1;
			u[3].score = (1-term1)/norm3;
			u[3].firstMember = (1-term1)/(norm3*norm3);

			for (i=0;i<nrRec;i++)
				u[3].alpha[i] = u[3].firstMember * (-kprime[i].neg);
		}

		// make the score in [0,1] interval
		pvm_float scoreSum=0, minScoreSum = 1e-10f;
		int nrParticipants=0;
		for (i=0;i<4;i++) {
			scoreSum += u[i].score;
			if (u[i].infeas_eq) nrParticipants++;
		}

		// search for max score over this block for stop condition
		maxScore = FLT_MIN;
		for (i=0;i<4;i++) {
			if (maxScore < u[i].score) {
				maxScore = u[i].score;
			}
		}

		if (scoreSum < minScoreSum)
			scoreSum = minScoreSum;
		
		for (i = 0; i < 4; i++)
			u[i].score /= scoreSum;			

		// we use as output the exact same buffers used for input
		pvm_float mean;
		for (i=0;i<nrRec;i++) {
			// compute for alphas
			mean = 0;
			for (int j=0;j<4;j++) {
				if (u[j].infeas_eq) { // if the eq was not satisfied and it will contribute to the update
					mean += u[j].alpha[i]*u[j].score;
				}
			}
			alpha[i] += mean;

			// compute for sigmas
			con->GetRecordLabel(label, i);
			mean = 0;
			if (u[0].infeas_eq) {
				if (label==1) mean += u[0].firstMember * u[0].sigmaVal * u[0].score;
			}
			if (u[1].infeas_eq) {
				if (label!=1) mean += u[1].firstMember * u[1].sigmaVal * u[1].score;
			}
			sigma[i] += mean;
		}

		mean = 0;
		for (i=0;i<4;i++) {
			if (u[i].infeas_eq) {
				mean += u[i].firstMember * u[i].b * u[i].score;
			}
		}
		b += mean;	

		for (i = 0; i < 4; i++)
			u[i].reset(nrRec);
	}
	// write update to disk
	fileName.Truncate(0);
	fileName.AddFormated("%s.state.iter.%03d.block.last", varBlockFilePrefix.GetText(), varIterNr);
	CHECKMSG(fileObj.Create(fileName), "could not create file: %s", fileName.GetText());

	// write state header	
	stateHeader.blkNr = -1;
	stateHeader.recCount = nrRec;
	stateHeader.recStart = 0;
	stateHeader.totalRecCount = nrRec;

	// write alphas
	CHECKMSG(fileObj.Write(&stateHeader, sizeof(StateFileHeader), &written), "could not write to file");
	CHECKMSG(sizeof(StateFileHeader)==written,"could not write enough to file");

	// write alphas
	CHECKMSG(fileObj.Write(alpha, vectSz, &written), "could not write to file");
	CHECKMSG(vectSz==written,"could not write enough to file");

	// write sigmas
	CHECKMSG(fileObj.Write(sigma, vectSz, &written), "could not write to file");
	CHECKMSG(vectSz==written,"could not write enough to file");

	// write b
	CHECKMSG(fileObj.Write(&b, sizeof(pvm_float), &written), "could not write to file");
	CHECKMSG(sizeof(pvm_float)==written,"could not write enough to file");

	// write score for this block
	CHECKMSG(fileObj.Write(&maxScore, sizeof(pvm_float), &written), "could not write to file");
	CHECKMSG(sizeof(pvm_float)==written,"could not write enough to file");


	fileObj.Close();

	return true;			
}

void ProbVectorMachine::UpdateStr::reset(int recCount)
{
	sigmaVal = b = score = firstMember = 0;
	infeas_eq = false;
	if (alpha)
		memset(alpha, 0, recCount * sizeof(pvm_float));	
}

bool ProbVectorMachine::DumpDefaultStateVariables()
{
	UInt32 nrRec = con->GetRecordCount();
	UInt32 vectSz = sizeof(pvm_float)*nrRec;
	UInt64 written;

	GML::Utils::File				fileObj;
	GML::Utils::GString				fileName;

	pvm_float *alpha, *sigma;

	alpha = (pvm_float*) malloc(vectSz);
	NULLCHECKMSG(alpha, "could not alloc memory for alphaOrig");

	sigma = (pvm_float*) malloc(vectSz);
	NULLCHECKMSG(sigma, "could not alloc memory for sigmaPOrig");

	// initialize state variables with values read from disk or init here
	fileName.Truncate(0);
	fileName.AddFormated("%s.state.iter.000.block.all", varBlockFilePrefix.GetText());

	UInt32 i;

	for (i = 0; i < nrRec; i++) {
		alpha[i] = 1;
		sigma[i] = 0;
	}

	CHECKMSG(fileObj.Create(fileName), "could not create file: %s", fileName.GetText());

	StateFileHeader stateHeader;
	stateHeader.blkNr = -1;
	stateHeader.recStart = 0;
	stateHeader.recCount = nrRec;
	stateHeader.totalRecCount = nrRec;

	// write header
	CHECKMSG(fileObj.Write(&stateHeader, sizeof(StateFileHeader),&written), "could not write to file");
	CHECKMSG(written==sizeof(StateFileHeader), "could not write enough to file");

	// write alphas
	CHECKMSG(fileObj.Write(alpha, vectSz,&written), "could not write to file");
	CHECKMSG(written==vectSz, "could not write enough to file");

	// write sigmas
	CHECKMSG(fileObj.Write(sigma, vectSz,&written), "could not write to file");
	CHECKMSG(written==vectSz, "could not write enough to file");

	// write b
	pvm_float b = 0.0;
	CHECKMSG(fileObj.Write(&b, sizeof(pvm_float),&written), "could not write to file");
	CHECKMSG(written==sizeof(pvm_float), "could not write enough to file");

	free(alpha);
	free(sigma);

	INFOMSG("Default variables dumped to disk succesfully");

	return true;
}

bool ProbVectorMachine::GatherBlockStates()
{
	UInt64 read, written, nrBlocks;
	UInt32 i;
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	StateFileHeader stateHeader;

	UInt32 nrRec = con->GetRecordCount();
	UInt32 vectSz = sizeof(pvm_float)*nrRec;

	GML::Utils::File				fileObj;
	GML::Utils::GString				fileName;

	pvm_float *alpha, *sigma, *alphaMean, *sigmaMean;
	pvm_float scoreSum, score;

	UInt32 *alpha_count, *sigma_count;

	alpha = (pvm_float*) malloc(vectSz);
	NULLCHECKMSG(alpha, "could not alloc memory");
	alphaMean = (pvm_float*) malloc(vectSz);
	NULLCHECKMSG(alphaMean, "could not alloc memory");

	sigma = (pvm_float*) malloc(vectSz);
	NULLCHECKMSG(sigma, "could not alloc memory");
	sigmaMean = (pvm_float*) malloc(vectSz);
	NULLCHECKMSG(sigmaMean, "could not alloc memory");

	alpha_count = (UInt32 *)malloc(nrRec * sizeof(UInt32));
	NULLCHECKMSG(alpha_count, "could not alloc memory");	
	sigma_count = (UInt32 *)malloc(nrRec * sizeof(UInt32));
	NULLCHECKMSG(sigma_count, "could not alloc memory");	

	memset(alphaMean, 0, vectSz);
	memset(sigmaMean, 0, vectSz);

	memset(alpha_count, 0, nrRec * sizeof(UInt32));
	memset(sigma_count, 0, nrRec * sizeof(UInt32));

	fileName.Truncate(0);
	fileName.AddFormated("%s.state.iter.%03d.block.???", varBlockFilePrefix.GetText(), varIterNr);

	nrBlocks = 0;
	scoreSum = 0;
	hFind = FindFirstFile(fileName.GetText(), &FindFileData);
	do {
		INFOMSG(FindFileData.cFileName);
		if (strstr(FindFileData.cFileName, ".all")==NULL) {
			nrBlocks ++;

			// read the file
			CHECKMSG(fileObj.OpenRead(FindFileData.cFileName), "could not open file:%s for reading", FindFileData.cFileName);
			
			CHECKMSG(fileObj.Read(&stateHeader, sizeof(StateFileHeader), &read), "could not read from file: %s", FindFileData.cFileName);
			CHECKMSG(read==sizeof(StateFileHeader), "could not read enough from file");
			
			CHECKMSG(fileObj.Read(alpha, vectSz, &read),"could not read from file: %s", FindFileData.cFileName);
			CHECKMSG(read==vectSz, "could not read enough from file");
			
			CHECKMSG(fileObj.Read(sigma, vectSz, &read),"could not read from file: %s", FindFileData.cFileName);
			CHECKMSG(read==vectSz, "could not read enough from file");

			CHECKMSG(fileObj.Read(&score, sizeof(pvm_float), &read),"could not read from file: %s", FindFileData.cFileName);
			CHECKMSG(read==sizeof(pvm_float), "could not read enough from file");

			fileObj.Close();

			// sum scores for stop condition
			scoreSum += score*score;

			for (i=0;i<nrRec;i++) 
			{
				alphaMean[i] += alpha[i];
				alpha_count[i]++;
			}
			
			for (i=stateHeader.recStart;i<stateHeader.recStart + stateHeader.recCount;i++)
			{
				sigmaMean[i] += sigma[i];			
				sigma_count[i]++;
			}
		}

	} while (FindNextFile(hFind, &FindFileData));

	pvm_float b;

	fileName.Truncate(0);
	fileName.AddFormated("%s.state.iter.%03d.block.last", varBlockFilePrefix.GetText(), varIterNr);
	// read the file
	CHECKMSG(fileObj.OpenRead(fileName.GetText()), "could not open file:%s for reading", fileName.GetText());
	
	CHECKMSG(fileObj.Read(&stateHeader, sizeof(StateFileHeader), &read), "could not read from file: %s", fileName.GetText());
	CHECKMSG(read==sizeof(StateFileHeader), "could not read enough from file");
	
	CHECKMSG(fileObj.Read(alpha, vectSz, &read),"could not read from file: %s", fileName.GetText());
	CHECKMSG(read==vectSz, "could not read enough from file");

	CHECKMSG(fileObj.Read(sigma, vectSz, &read),"could not read from file: %s", fileName.GetText());
	CHECKMSG(read==vectSz, "could not read enough from file");

	CHECKMSG(fileObj.Read(&b, sizeof(pvm_float), &read),"could not read from file: %s", fileName.GetText());
	CHECKMSG(read==sizeof(pvm_float), "could not read enough from file");

	CHECKMSG(fileObj.Read(&score, sizeof(pvm_float), &read),"could not read from file: %s", fileName.GetText());
	CHECKMSG(read==sizeof(pvm_float), "could not read enough from file");

	fileObj.Close();

	// sum scores for stop condition
	scoreSum += score*score;
	scoreSum = sqrt(scoreSum);

	for (i=0;i<nrRec;i++) {
		alphaMean[i] += nrBlocks * alpha[i];
		alphaMean[i] /= (pvm_float)(nrBlocks + alpha_count[i]);
		CHECKMSG(alpha_count[i] > 0, "unrepresented alpha");
		//alphaMean[i] /= nrBlocks+1;
		//CHECKMSG(nrBlocks == alpha_count[i], "dubios number of alphas");
	}

	for (i=0;i<nrRec;i++) {
		sigmaMean[i] += nrBlocks * sigma[i];
		sigmaMean[i] /= (pvm_float)(nrBlocks + sigma_count[i]);
		CHECKMSG(sigma_count[i] > 0, "unrepresented sigma");
		//sigmaMean[i] /= 2;
		//CHECKMSG(1 == sigma_count[i], "dubios number of sigmas");
	}

	fileName.Truncate(0);
	fileName.AddFormated("%s.state.iter.%03d.block.all", varBlockFilePrefix.GetText(), varIterNr);

	CHECKMSG(fileObj.Create(fileName.GetText()),"could not create file:%s ", fileName.GetText());
	
	CHECKMSG(fileObj.Write(&stateHeader, sizeof(StateFileHeader), &written), "could not write to file");
	CHECKMSG(written==sizeof(StateFileHeader), "could not write enough to file");

	CHECKMSG(fileObj.Write(alphaMean, vectSz, &written), "could not write to file");
	CHECKMSG(written==vectSz, "could not write enough to file");

	CHECKMSG(fileObj.Write(sigmaMean, vectSz, &written), "could not write to file");
	CHECKMSG(written==vectSz, "could not write enough to file");

	CHECKMSG(fileObj.Write(&b, sizeof(pvm_float), &written), "could not write to file");
	CHECKMSG(written==sizeof(pvm_float), "could not write enough to file");
	fileObj.Close();

	// make string representatin of my score
	GML::Utils::GString scoreStr;
	scoreStr.Truncate(0);
	scoreStr.AddFormated("%.02f\n",scoreSum);

	// write string to .score file as text
	fileName.Truncate(0);
	fileName.AddFormated("%s.state.iter.%03d.block.score", varBlockFilePrefix.GetText(), varIterNr);
	CHECKMSG(fileObj.Create(fileName.GetText()),"could not create file:%s ", fileName.GetText());	
	CHECKMSG(fileObj.Write(scoreStr.GetText(), sizeof(char)*(scoreStr.GetSize()-1), &written), "could not write to file");
	CHECKMSG(written==sizeof(char)*(scoreStr.GetSize()-1), "could not write enough to file");
	fileObj.Close();

	return true;
}

bool ProbVectorMachine::ClasifyDataset()
{
	UInt64 read;
	UInt32 i, j;

	StateFileHeader stateHeader;

	UInt32 nrRec = con->GetRecordCount();
	UInt32 vectSz = sizeof(pvm_float)*nrRec;

	GML::Utils::File				fileObj;

	pvm_float *alpha, *sigma, b;

	// sanity checks
	UInt32 trainNrFeat, testNrFeat;
	trainNrFeat = con->GetFeatureCount();
	testNrFeat  = conectorTest->GetFeatureCount();
	CHECKMSG(trainNrFeat == testNrFeat, "the test dataset has to have the same number of features as the train dataset");

	// read data from model file
	alpha = (pvm_float*) malloc(vectSz);
	NULLCHECKMSG(alpha, "could not alloc memory");

	sigma = (pvm_float*) malloc(vectSz);
	NULLCHECKMSG(sigma, "could not alloc memory");

	CHECKMSG(fileObj.OpenRead(varModelFile.GetText()), "could not open file:%s for reading", varModelFile.GetText());

	CHECKMSG(fileObj.Read(&stateHeader, sizeof(StateFileHeader), &read), "could not read from file: %s", varModelFile.GetText());
	CHECKMSG(read==sizeof(StateFileHeader), "could not read enough from file");

	CHECKMSG(fileObj.Read(alpha, vectSz, &read),"could not read from file: %s", varModelFile.GetText());
	CHECKMSG(read==vectSz, "could not read enough from file");

	CHECKMSG(fileObj.Read(sigma, vectSz, &read),"could not read from file: %s", varModelFile.GetText());
	CHECKMSG(read==vectSz, "could not read enough from file");

	CHECKMSG(fileObj.Read(&b, sizeof(pvm_float), &read),"could not read from file: %s", varModelFile.GetText());
	CHECKMSG(read==sizeof(pvm_float), "could not read enough from file");

	fileObj.Close();

	UInt32 nrRecordsTest = conectorTest->GetRecordCount();
	UInt32 nrTestPos, nrTestNeg, nrFalsePos, nrFalseNeg;
	pvm_float ker, result;
	double label;
	GML::ML::MLRecord  mlRecTest, mlRecTrain;
	ker_f_wrapper kernCompute(con, notif);
	
	kernCompute.set_params(varKernelParamDouble, varKernelParamInt, NULL, (KerFuncType)varKernelType);

	nrFalseNeg = nrFalsePos = 0;
	nrTestNeg  = nrTestPos  = 0;

	CHECKMSG(conectorTest->CreateMlRecord(mlRecTest),"could not create the test ml record");
	CHECKMSG(con->CreateMlRecord(mlRecTrain),"could not create the train ml record");

	for (i=0;i<nrRecordsTest;i++) {
		CHECKMSG(conectorTest->GetRecord(mlRecTest, i), "could not get test record at index: %d", i);
		
		// check label
		CHECKMSG(conectorTest->GetRecordLabel(label, i), "could not get test record label at index: %d", i);
		if (label == 1) nrTestPos++;
		else nrTestNeg++;

		// compute 

		result = b;
		for (j=0;j<nrRec;j++) {
			CHECKMSG(con->GetRecord(mlRecTrain, j), "could not get train record at index: %d", j);
			
			ker = (pvm_float)kernCompute.compute_for(mlRecTest, mlRecTrain);
			result += alpha[j] * ker;
		}

		// compare with original label
		if (label==1 && result<0) nrFalseNeg++;
		if (label!=1 && result>0) nrFalsePos++;
	}

	INFOMSG("TP : %.02f",(nrTestPos-nrFalseNeg)/nrTestPos*100);
	INFOMSG("TN : %.02f",(nrTestNeg-nrFalsePos)/nrTestNeg*100);

	INFOMSG("FP : %.02f",(nrFalsePos)/nrTestNeg*100);
	INFOMSG("FN : %.02f",(nrFalseNeg)/nrTestPos*100);

	INFOMSG("ACC: %.02f",(nrRecordsTest-(nrFalseNeg+nrFalsePos))/nrRecordsTest*100);

	return true;
}

bool ProbVectorMachine::InitExtraConnections()
{
	if ((conectorTest = GML::Builder::CreateConnector(varConectorTest.GetText(),*notif))==NULL)
	{
		notif->Error("[%s] -> Unable to create Conector (%s)",ObjectName,varConectorTest.GetText());
		return false;
	}
	return true;
}
