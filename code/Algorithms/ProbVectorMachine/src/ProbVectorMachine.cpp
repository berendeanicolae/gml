#include "ProbVectorMachine.h"
#include "KernelFunctionDBG.h"

#include "KernelWrapper.h"

ProbVectorMachine::ProbVectorMachine()
{
	ObjectName = "ProbVectorMachine";

	//Add extra commands here
	SetPropertyMetaData("Command","!!LIST:None=0,DebugTest,PreCompGramMatrix,MergeKprimeFiles,PreCompEqNorm,InitStateVars,BlockTraining,LastBlockTraining!!");
	
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
	LinkPropertyToDouble("T",(double&)varT,1.0,"The T algorithm parameter; used for binary search");
	LinkPropertyToUInt32("WindowSize",varWindowSize,10,"The Windows size the block solver works with");
	LinkPropertyToUInt32("IterationNo",varIterNr,0,"The current iteration number");
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
		case COMMAND_INIT_STATE_VARS:
			INFOMSG("Dumping default state variables to disk");
			DumpDefaultStateVariables();
			break;
		case COMMAND_LAST_BLOCK_TRAINING:
			INFOMSG("Starting the training for the last block");
			LastBlockTraining();
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
					wu.uALPH[uALPH_it] = frac * (kpr-ker);
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

bool ProbVectorMachine::LastBlockTraining()
{
	UInt32 nrRec = con->GetRecordCount(), i;
	UInt32 vectSz = sizeof(pvm_float)*nrRec;
	UInt64 read, written;

	PreCache::KPrimePair	  *kprime;

	GML::Utils::File				fileObj;
	GML::Utils::GString				fileName;
	PreCache::PreCacheFileHeader	kpHeader;

	pvm_float norm0, norm1, norm2, norm3, term0, term1;

	pvm_float *alpha, *sigma, b;

	alpha = (pvm_float*) malloc(vectSz);
	NULLCHECKMSG(alpha, "could not alloc memory for alphaOrig");

	sigma = (pvm_float*) malloc(vectSz);
	NULLCHECKMSG(sigma, "could not alloc memory for sigmaPOrig");

	// initialize state variables with values read from disk or init here
	fileName.Truncate(0);
	fileName.AddFormated("%s.state.iter.%03d.block.all", varBlockFilePrefix.GetText(), varIterNr-1);

	// read state variables from disk
	CHECKMSG(fileObj.OpenRead(fileName),"could not open file for reading: %s",fileName.GetText());

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
	
	norm0 = varT*varT * (Splus-1)*(Splus-1) * norm2 + Splus/((Splus-1)*(Splus-1));
	norm1 = varT*varT * (Sminus-1)*(Sminus-1) * norm3 + Sminus/((Sminus-1)*(Sminus-1));

	term0 = term1 = b;
	for (i=0;i<nrRec;i++) {
		term0 += alpha[i]*kprime[i].pos;
		term1 += alpha[i]*kprime[i].neg;
	}

	pvm_float sumSigmaPlus = 0;
	pvm_float sumSigmaMinus = 0;
	for (i=0;i<nrRec;i++) {
		CHECKMSG(con->GetRecordLabel(label, i), "could not get record:%d label", i);
		if (label==1)
			sumSigmaPlus += sigma[i];
		else
			sumSigmaMinus += sigma[i];
	}

	pvm_float s[4];
	UpdateStr u[4];
	memset(u, 0, sizeof(UpdateStr)*4);
	memset(s, 0, sizeof(pvm_float)*4);

	// first equation

	s[0] = varT * (Splus-1)* term0 - sumSigmaPlus;
	
	if (s[0]<0) {
		u[0].alpha = (pvm_float*) malloc(nrRec*sizeof(pvm_float));
		CHECKMSG(u[0].alpha, "could not alloc memory");
		u[0].sigmaVal = -1;
		u[0].b = 1;
		u[0].score = -s[0]/norm0;
		u[0].firstMember = -s[0]/(norm0*norm0);

		for (i=0;i<nrRec;i++)
		   u[0].alpha[i] = u[0].firstMember * kprime[i].pos;
	}

	// second equation

	s[1] = varT * (Sminus-1)* term1 - sumSigmaMinus;
	if (s[1]<0) {
		u[1].alpha = (pvm_float*) malloc(nrRec*sizeof(pvm_float));
		CHECKMSG(u[1].alpha, "could not alloc memory");
		u[1].sigmaVal = -1;
		u[1].b = -1;
		u[1].score = -s[1]/norm1;
		u[1].firstMember = -s[1]/(norm1*norm1);

		for (i=0;i<nrRec;i++)
			u[1].alpha[i] = u[1].firstMember * (-kprime[i].neg);
	}

	// third equation

	if (term0<0) {
		u[2].alpha = (pvm_float*) malloc(nrRec*sizeof(pvm_float));
		CHECKMSG(u[2].alpha, "could not alloc memory");
		u[2].sigmaVal = 0;
		u[2].b = 1;
		u[2].score = (1-term0)/norm2;
		u[2].firstMember = (1-term0)/(norm2*norm2);

		for (i=0;i<nrRec;i++)
			u[2].alpha[i] = u[2].firstMember * (kprime[i].pos);
	}

	// forth equation

	if (term1<0) {
		u[3].alpha = (pvm_float*) malloc(nrRec*sizeof(pvm_float));
		CHECKMSG(u[3].alpha, "could not alloc memory");
		u[3].sigmaVal = 0;
		u[3].b = -1;
		u[3].score = (1-term1)/norm3;
		u[3].firstMember = (1-term1)/(norm3*norm3);

		for (i=0;i<nrRec;i++)
			u[3].alpha[i] = u[3].firstMember * (-kprime[i].neg);
	}

	// make the score in [0,1] interval
	pvm_float scoreSum=0;
	int nrParticipants=0;
	for (i=0;i<4;i++) {
		scoreSum += u[i].score;
		if (u[i].alpha!=NULL) nrParticipants++;
	}

	for (i=0;i<4;i++) {
		u[i].score = u[i].score/scoreSum;
	}

	// we use as output the exact same buffers used for input
	pvm_float mean;
	for (i=0;i<nrRec;i++) {
		// compute for alphas
		mean = 0;
		for (int j=0;j<4;j++) {
			if (u[j].alpha!=NULL) { // if the eq was not satisfied and it will contribute to the update
				mean += u[j].alpha[i]*u[j].score;
			}
		}
		alpha[i] = mean/nrParticipants;

		// compute for sigmas
		con->GetRecordLabel(label, i);
		mean = 0;
		if (u[0].alpha!=NULL) {
			if (label==1) mean += u[0].firstMember * u[0].sigmaVal;
		}
		if (u[1].alpha!=NULL) {
			if (label!=1) mean += u[1].firstMember * u[1].sigmaVal;
		}
		sigma[i] = mean/nrParticipants;
	}

	mean = 0;
	for (i=0;i<4;i++) {
		if (u[i].alpha!=NULL) {
			mean += u[i].b;
		}
	}
	b = mean/nrParticipants;

	// write update to disk
	fileName.Truncate(0);
	fileName.AddFormated("%s.state.iter.%03d.block.last", varBlockFilePrefix.GetText(), varIterNr);
	CHECKMSG(fileObj.Create(fileName), "could not create file: %s", fileName.GetText());

	// write alphas
	CHECKMSG(fileObj.Write(alpha, vectSz, &written), "could not write to file");
	CHECKMSG(vectSz==written,"could not write enough to file");

	// write sigmas
	CHECKMSG(fileObj.Write(sigma, vectSz, &written), "could not write to file");
	CHECKMSG(vectSz==written,"could not write enough to file");

	// write b
	CHECKMSG(fileObj.Write(alpha, vectSz, &written), "could not write to file");
	CHECKMSG(vectSz==written,"could not write enough to file");

	fileObj.Close();

	return true;			
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

	UInt32 i, last_i = nrRec / 2;

	for (i = 0; i < last_i; i++)
		alpha[i] = 1;

	for (i = last_i; i < nrRec; i++)
		alpha[i] = -0.5;

	memset(sigma, 0, vectSz);

	CHECKMSG(fileObj.Create(fileName), "could not create file: %s", fileName.GetText());

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
