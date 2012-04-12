#include "ProbVectorMachine.h"
#include "KernelFunctionDBG.h"

#include "KernelWrapper.h"

ProbVectorMachine::ProbVectorMachine()
{
	ObjectName = "ProbVectorMachine";

	//Add extra commands here
	SetPropertyMetaData("Command","!!LIST:None=0,DebugTest,PreCompGramMatrix,MergeKprimeFiles,PreCompEqNorm,BlockTraining!!");
	
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
	LinkPropertyToString("AlgoIterationState",varAlgoIterationState, "", "File from where the current iteration state will be read or none if it's the first iteration");
	LinkPropertyToDouble("Lambda",varLambda,1.0,"The Lambda parameter for this iteration");
	LinkPropertyToUInt32("WindowSize",varWindowSize,10,"The Windows size the block solver works with");
	LinkPropertyToUInt32("IterNr",varIterNr,0,"The current iteration number");
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

void ProbVectorMachine::PreCacheInstInit()
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
	
	PreCacheInstInit();

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

	// alloc data for the original state variables

	UInt32 nrRec = con->GetRecordCount();
	UInt32 vectSz = sizeof(pvm_float)*nrRec;
	UInt64 read, written;

	PreCache::BlockLoadHandle *handle;
	PreCache::KPrimePair	  *kprime;
	
	GML::Utils::File				fileObj;
	GML::Utils::GString				fileName;
	PreCache::PreCacheFileHeader	kpHeader;

	pvm_float *alpha, *sigma;
	
	alpha = (pvm_float*) malloc(vectSz);
	NULLCHECKMSG(alpha, "could not alloc memory for alphaOrig");

	sigma = (pvm_float*) malloc(vectSz);
	NULLCHECKMSG(sigma, "could not alloc memory for sigmaPOrig");

	// initialize state variables with values read from disk or init here
	fileName.Truncate(0);
	fileName.AddFormated("%s.state.iter.%03d.block.all", varBlockFilePrefix.GetText(), varIterNr-1);

	if (!fileObj.OpenRead(fileName)) {
		// this is the first iteration, we have no state variables
		// set alphas to 1 according to andrei's code review
		//for (UInt32 i=0;i<nrRec;i++)
			//alpha[i] = 1;

		UInt32 i, last_i = nrRec / 2;

		for (i = 0; i < last_i; i++)
			alpha[i] = 1;

		for (i = last_i; i < nrRec; i++)
			alpha[i] = -0.5;

		memset(sigma, 0, vectSz);	
		
	} else {
		// read state variables from disk
		CHECKMSG(fileObj.OpenRead(varAlgoIterationState),"could not open file for reading: %s",varAlgoIterationState.GetText());
		
		CHECKMSG(fileObj.Read(alpha, vectSz, &read), "could not read from state file");
		CHECKMSG(read==vectSz, "could not read enough from state file");

		CHECKMSG(fileObj.Read(sigma, vectSz, &read), "could not read from state file");
		CHECKMSG(read==vectSz, "could not read enough from state file");
	}
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

	PreCacheInstInit();
	InstPreCache.AtInitLoading();

	// signal the first load because the for will start by waiting
	InstPreCache.AtSignalStartLoading(varBlockStart);

	for (UInt32 blkIdx=varBlockStart; blkIdx<varBlockStart+varBlockCount; blkIdx++)
	{
		handle = InstPreCache.AtWaitForCompletion();						
		NULLCHECKMSG(handle,"could not load block file:%d exiting", blkIdx-1);		
		handle->KPRM = kprime;

		CHECKMSG(InstPreCache.AtSignalStartLoading(blkIdx),"could not signal loading of block:%d ", blkIdx);
		CHECKMSG(PerfomBlockTraining(blkIdx, handle), "error performing training on block: %d", blkIdx);
	}

	// dump the block state variables to disk	
	fileName.Truncate(0);	
	fileName.AddFormated("%s.state.iter.%03d.block.%02d", varBlockFilePrefix.GetText(), varIterNr, handle->blkNr);

	CHECKMSG(fileObj.Create(fileName), "could not create file: %s", fileName.GetText());
	
	CHECKMSG(fileObj.Write(wu.ALPH,vectSz,&written),"could not write to block state file");
	CHECKMSG(written==vectSz,"could not write enough to the block state file");

	CHECKMSG(fileObj.Write(wu.SIGM,vectSz,&written),"could not write to block state file");
	CHECKMSG(written==vectSz,"could not write enough to the block state file");
	fileObj.Close();

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
	UInt32	  i, w, k;

	UInt32 nrRec = con->GetRecordCount();

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
		for (i=0;i<wu.winSize;i++)		
			scoreSum += wu.san[i];

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
	}

	return true;
}

bool ProbVectorMachine::PerformWindowUpdate(GML::Algorithm::MLThreadData &thData)
{
	UInt32		recIdxGlob, recIdxBlock, i, uALPH_it;
	double		label;
	pvm_float	sj, ker, kpr, frac;
	PreCache::KPrimePair  *wu_bHandle_KPRM;
	pvm_float   *wu_ALPH;


	INFOTHDMSG("perform window update");
	
	UInt32 nrRec = con->GetRecordCount();
	wu.updateNeeded = false;
	/*
	for (UInt32 winIt=thData.ThreadID; winIt<wu.winSize; winIt+=threadsCount)
	{
		recIdxBlock = wu.winStart + winIt;
		recIdxGlob = wu.bHandle->recStart + recIdxBlock; 		
		
		CHECKMSG(con->GetRecordLabel(label, recIdxGlob),"could not get record label");				
		
		sj = 0;

		if (label == 1)
		{
			for (i = 0; i < nrRec; i++) 
			{
				ker = KerAt(recIdxBlock, i, wu.bHandle->KERN, nrRec);
				kpr = wu.bHandle->KPRM[i].pos;
				sj += wu.ALPH[i] * (ker - kpr);
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
		}
		

		if (label==1) {
			if (wu.SIGM[recIdxGlob] - sj < 0) 
			{
				wu.updateNeeded = true;
				frac = (sj - wu.SIGM[recIdxGlob]) / (wu.bHandle->NORM[recIdxBlock]*wu.bHandle->NORM[recIdxBlock]);
				for (i=0;i<nrRec;i++) {
					ker = KerAt(recIdxBlock, i, wu.bHandle->KERN, nrRec);				
					kpr = wu.bHandle->KPRM[i].pos;
					wu.uALPH[winIt*nrRec + i] = frac * (ker-kpr);
				}
				wu.uSIGM[winIt] = frac;
				wu.san[winIt]	= (sj - wu.SIGM[recIdxGlob])/wu.bHandle->NORM[recIdxBlock];
			} else if (wu.SIGM[recIdxGlob] + sj < 0) 
			{
				wu.updateNeeded = true;
				frac = (-sj - wu.SIGM[recIdxGlob]) / (wu.bHandle->NORM[recIdxBlock]*wu.bHandle->NORM[recIdxBlock]);
				for (i=0;i<nrRec;i++) {
					ker = KerAt(recIdxBlock, i, wu.bHandle->KERN, nrRec);
					kpr = wu.bHandle->KPRM[i].pos;
					wu.uALPH[winIt*nrRec + i] = frac * (kpr-ker);
				}
				wu.uSIGM[winIt] = frac;
				wu.san[winIt]	= (-sj - wu.SIGM[recIdxGlob])/wu.bHandle->NORM[recIdxBlock];
			} 
		} else {
			if (wu.SIGM[recIdxGlob] - sj < 0) 
			{
				wu.updateNeeded = true;
				frac = (sj - wu.SIGM[recIdxGlob]) / (wu.bHandle->NORM[recIdxBlock]*wu.bHandle->NORM[recIdxBlock]);
				for (i=0;i<nrRec;i++) {
					ker = KerAt(recIdxBlock, i, wu.bHandle->KERN, nrRec);				
					kpr = wu.bHandle->KPRM[i].neg;
					wu.uALPH[winIt*nrRec + i] = frac * (ker-kpr);
				}
				wu.uSIGM[winIt] = frac;
				wu.san[winIt]	= (sj - wu.SIGM[recIdxGlob])/wu.bHandle->NORM[recIdxBlock];
			} else if (wu.SIGM[recIdxGlob] + sj < 0) 
			{
				wu.updateNeeded = true;
				frac = (-sj - wu.SIGM[recIdxGlob]) / (wu.bHandle->NORM[recIdxBlock]*wu.bHandle->NORM[recIdxBlock]);
				for (i=0;i<nrRec;i++) {
					ker = KerAt(recIdxBlock, i, wu.bHandle->KERN, nrRec);
					kpr = wu.bHandle->KPRM[i].neg;
					wu.uALPH[winIt*nrRec + i] = frac * (kpr-ker);
				}
				wu.uSIGM[winIt] = frac;
				wu.san[winIt]	= (-sj - wu.SIGM[recIdxGlob])/wu.bHandle->NORM[recIdxBlock];
			} 
		}
	}*/	
	for (UInt32 winIt=thData.ThreadID; winIt<wu.winSize; winIt+=threadsCount)
	{
		recIdxBlock = wu.winStart + winIt;
		recIdxGlob = wu.bHandle->recStart + recIdxBlock; 		
		
		CHECKMSG(con->GetRecordLabel(label, recIdxGlob),"could not get record label");				
		
		sj = 0;

		if (label == 1)
		{
			wu_ALPH = wu.ALPH;
			wu_bHandle_KPRM = wu.bHandle->KPRM;
			for (i = 0; i < nrRec; i++, wu_ALPH++, wu_bHandle_KPRM++) 
			{
				ker = KerAt(recIdxBlock, i, wu.bHandle->KERN, nrRec);
				kpr = wu_bHandle_KPRM->pos;
				sj += (*wu_ALPH) * (ker - kpr);
			}

			if (wu.SIGM[recIdxGlob] - sj < 0) 
			{
				wu.updateNeeded = true;
				frac = (sj - wu.SIGM[recIdxGlob]) / (wu.bHandle->NORM[recIdxBlock]*wu.bHandle->NORM[recIdxBlock]);
				wu_bHandle_KPRM = wu.bHandle->KPRM;
				for (i = 0, uALPH_it = winIt*nrRec; i < nrRec; i++, uALPH_it++, wu_bHandle_KPRM++) 
				{
					ker = KerAt(recIdxBlock, i, wu.bHandle->KERN, nrRec);				
					kpr = wu_bHandle_KPRM->pos;
					wu.uALPH[uALPH_it] = frac * (ker-kpr);
				}
				wu.uSIGM[winIt] = frac;
				wu.san[winIt]	= (sj - wu.SIGM[recIdxGlob])/wu.bHandle->NORM[recIdxBlock];
			} else if (wu.SIGM[recIdxGlob] + sj < 0) 
			{
				wu.updateNeeded = true;
				frac = (-sj - wu.SIGM[recIdxGlob]) / (wu.bHandle->NORM[recIdxBlock]*wu.bHandle->NORM[recIdxBlock]);
				wu_bHandle_KPRM = wu.bHandle->KPRM;
				for (i = 0, uALPH_it = winIt*nrRec; i < nrRec; i++, uALPH_it++, wu_bHandle_KPRM++) 
				{
					ker = KerAt(recIdxBlock, i, wu.bHandle->KERN, nrRec);
					kpr = wu_bHandle_KPRM->pos;
					wu.uALPH[uALPH_it + i] = frac * (kpr-ker);
				}
				wu.uSIGM[winIt] = frac;
				wu.san[winIt]	= (-sj - wu.SIGM[recIdxGlob])/wu.bHandle->NORM[recIdxBlock];
			} 
		}
		else
		{
			wu_ALPH = wu.ALPH;
			wu_bHandle_KPRM = wu.bHandle->KPRM;
			for (i = 0; i < nrRec; i++, wu_ALPH++, wu_bHandle_KPRM++) 
			{							
				ker = KerAt(recIdxBlock, i, wu.bHandle->KERN, nrRec);
				kpr = wu_bHandle_KPRM->neg;
				sj += (*wu_ALPH) * (ker - kpr);
			}

			if (wu.SIGM[recIdxGlob] - sj < 0) 
			{
				wu.updateNeeded = true;
				frac = (sj - wu.SIGM[recIdxGlob]) / (wu.bHandle->NORM[recIdxBlock]*wu.bHandle->NORM[recIdxBlock]);
				wu_bHandle_KPRM = wu.bHandle->KPRM;
				for (i = 0, uALPH_it = winIt*nrRec; i < nrRec; i++, uALPH_it++, wu_bHandle_KPRM++) 
				{
					ker = KerAt(recIdxBlock, i, wu.bHandle->KERN, nrRec);				
					kpr = wu_bHandle_KPRM->neg;
					wu.uALPH[uALPH_it] = frac * (ker-kpr);
				}
				wu.uSIGM[winIt] = frac;
				wu.san[winIt]	= (sj - wu.SIGM[recIdxGlob])/wu.bHandle->NORM[recIdxBlock];
			} else if (wu.SIGM[recIdxGlob] + sj < 0) 
			{
				wu.updateNeeded = true;
				frac = (-sj - wu.SIGM[recIdxGlob]) / (wu.bHandle->NORM[recIdxBlock]*wu.bHandle->NORM[recIdxBlock]);
				wu_bHandle_KPRM = wu.bHandle->KPRM;
				for (i = 0, uALPH_it = winIt*nrRec; i < nrRec; i++, uALPH_it++) 
				{
					ker = KerAt(recIdxBlock, i, wu.bHandle->KERN, nrRec);
					kpr = wu_bHandle_KPRM->neg;
					wu.uALPH[uALPH_it] = frac * (kpr-ker);
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
