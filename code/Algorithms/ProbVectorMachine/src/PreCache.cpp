
#include "PreCache.h"
#include "ProbVectorMachine.h"

PreCache::PreCache()
{
	this->ObjectName = "ProbVectorMachine";	

	AtKillThread = false;
	AtEventWorking = CreateEvent(NULL,TRUE,FALSE,NULL);
	AtEventWaiting = CreateEvent(NULL,TRUE,TRUE,NULL);

}

bool PreCache::SetInheritData(PreCache::InheritData &inhData)
{	
	this->id.con = inhData.con;
	this->id.notif = inhData.notif;
	
	this->id.varBlockCount = inhData.varBlockCount;
	this->id.varBlockFileSize = inhData.varBlockFileSize;
	this->id.varBlockStart = inhData.varBlockStart;
	
	this->id.varKernelParamDouble = inhData.varKernelParamDouble;
	this->id.varKernelParamInt = inhData.varKernelParamInt;
	this->id.varKernelType = inhData.varKernelType;
	
	this->id.varBlockFilePrefix.Truncate(0);
	this->id.varBlockFilePrefix.Add(inhData.varBlockFilePrefix);

	this->con = id.con;
	this->notif = id.notif;

	NrRec = id.con->GetRecordCount();
	NrFts = id.con->GetFeatureCount();

	return true;
}

bool PreCache::PreComputeGram()
{
	GML::Utils::File kernelFileObj, kprimeFileObj;
	GML::Utils::GString kernelFileName, kprimeFileName; 
	unsigned char *kernelBuffer, *writeBuffer;
	KPrimePair  *kprimeBuffer;
	unsigned long long sizeOfBlock, writtenSoFar, written;	
	PreCacheFileHeader pcfh;

	// number of posibive/negative elements
	NrPosRec = NrNegRec = 0;
	double label;
	for (UInt32 i=0;i<NrRec;i++) {
		CHECKMSG(con->GetRecordLabel(label, i), "error getting record label: %d", i);
		if (label == 1) NrPosRec++;
		else NrNegRec++;
	}

	SizePerLine = sizeof(pvm_float)*NrRec;
	//RecPerBlock = GetNrRecPerBlock(0, NrRec);
	RecPerBlock = GetNrRecPerBlockNonRecursive(NrRec);

	TotalNrBlocks = NrRec/RecPerBlock;
	if (NrRec%RecPerBlock)
		TotalNrBlocks++;

	if (id.varBlockCount>=TotalNrBlocks)
		id.varBlockCount = TotalNrBlocks;

	// allocate memory for output buffer
	kernelBuffer = (unsigned char*) pvm_malloc (id.varBlockFileSize*UNMEGA);
	NULLCHECKMSG(kernelBuffer, "could not allocate enough memory for kernel output buffer");
	pccb.KernelBuffer = (pvm_float*) kernelBuffer;
	memset(kernelBuffer, 0, id.varBlockFileSize*UNMEGA);

	kprimeBuffer = (KPrimePair*) pvm_malloc (RecPerBlock*sizeof(KPrimePair));
	NULLCHECKMSG(kprimeBuffer, "could not allocate enough memory for kprime output buffer");
	pccb.KPrimeBuffer = kprimeBuffer;
	memset(kprimeBuffer, 0, RecPerBlock*sizeof(KPrimePair));

	// records per Block, useful for computing where we are right now
	pccb.RecPerBlock = RecPerBlock;
	pccb.BlockNr = id.varBlockStart;

	// iterate through the Blocks that need to be computed
	for (UInt32 i=id.varBlockStart;i<id.varBlockStart+id.varBlockCount && i*RecPerBlock<NrRec;i++) {
		
		// create the current output file
		CHECKMSG(GetBlockFileName(i, FileTypeKernel, kernelFileName),"could not compose kernel file name");		
		CHECKMSG(GetBlockFileName(i, FileTypeKPrime, kprimeFileName),"could not compose kprime file name");		

		CHECKMSG(kernelFileObj.Create(kernelFileName), "could not create file: %s", kernelFileName.GetText());
		CHECKMSG(kprimeFileObj.Create(kprimeFileName), "could not create file: %s", kprimeFileName.GetText());

		// set the current Block number for the map operation
		pccb.CurrBlockNr = i;
		pccb.RecStart = i*RecPerBlock;
		if (i==TotalNrBlocks-1) pccb.RecCount = NrRec - i*RecPerBlock;
		else pccb.RecCount = RecPerBlock;
					  
		// run the actual map command on threads
		alg->ExecuteParalelCommand(ProbVectorMachine::THREAD_COMMAND_PRECOMPUTE_BLOCK);

		//
		// write the out buffer to the disk
		// 
		sizeOfBlock = GetSizeOfBlock(i);
		writeBuffer = kernelBuffer;

		// set the file header fields
		memset(pcfh.Magic, 0, PRECACHE_FILE_HEADER_MAGIC_SZ);		
		sprintf_s((char*)pcfh.Magic,strlen(PRECACHE_FILE_HEADER_MAGIC)+1,PRECACHE_FILE_HEADER_MAGIC);		

		pcfh.NrFeatures = NrFts;
		pcfh.NrRecordsTotal = NrRec;
		pcfh.NrRecords = pccb.RecCount;
		pcfh.RecordStart = i*RecPerBlock;		
		pcfh.BlockSize = sizeOfBlock;

		//write the header of the PreCache file
		CHECKMSG(kernelFileObj.Write(0, &pcfh, sizeof(PreCacheFileHeader), &written), "failed to write kernel values file header");

		// the write loop
		writtenSoFar = written;
		while (writtenSoFar < sizeOfBlock+sizeof(PreCacheFileHeader)) {			
			// figure how much to write to this block
			//if (SzOfBlock-WrittenSoFar < UNMEGA) BlockSize = SzOfBlock-WrittenSoFar;
			//else BlockSize = UNMEGA;
		
			// the write operation
			CHECKMSG(kernelFileObj.Write(writtenSoFar, writeBuffer, UNMEGA, &written), "failed to write kernel values buffer");
			writtenSoFar += written;
		}

		// set the magic field
		memset(pcfh.Magic, 0, PRECACHE_FILE_HEADER_MAGIC_SZ);
		sprintf_s((char*)pcfh.Magic,strlen(KPRIME_FILE_HEADER_MAGIC)+1,KPRIME_FILE_HEADER_MAGIC);
		
		// set the block size of the kprime buffer
		pcfh.BlockSize = pccb.RecCount*sizeof(KPrimePair);

		//write the header of the PreCache file
		CHECKMSG(kprimeFileObj.Write(0, &pcfh, sizeof(PreCacheFileHeader), &written), "failed to write kprime values file header");
		// write kprime buffer
		CHECKMSG(kprimeFileObj.Write(written, kprimeBuffer, pccb.RecCount*sizeof(KPrimePair), &written), "failed to write kprime buffer");

		kernelFileObj.Close();
		kprimeFileObj.Close();
		pccb.BlockNr ++;
	}

	// free the buffers
	free(kernelBuffer);
	free(kprimeBuffer);

	return true;
}

int PreCache::GetNrRecPerBlockNonRecursive(int MaxNr)
{
	UInt32 SzPerBlock = id.varBlockFileSize*UNMEGA;
	UInt32 SzForAll = (NrRec*NrRec - (NrRec*(NrRec-1))/2)*sizeof(pvm_float);

	// what if the database is smaller than UNMEGA
	if (SzForAll<SzPerBlock) return MaxNr;

	//obviously, we have to split
	//this requires from us to solve a second degree equation

	pvm_float c = (float)SzPerBlock / (float)sizeof(pvm_float);
	pvm_float b = NrRec + (pvm_float)0.5;
	pvm_float delta = b * b - 2 * c;

	if (delta < 0)//this should not happen, as it has already been previously checked
		return MaxNr;	

	return (UInt32)(b - sqrtf(delta));
}

int PreCache::GetNrRecPerBlock(int MinNr, int MaxNr)
{
	// binary search for the nr of records per Block
	UInt32 MidNr = MinNr + (MaxNr-MinNr)/2;
	UInt32 SzForMid = (MidNr*NrRec - (MidNr*(MidNr-1))/2)*sizeof(pvm_float);
	UInt32 SzPerBlock = id.varBlockFileSize*UNMEGA;
	UInt32 SzForAll = (NrRec*NrRec - (NrRec*(NrRec-1))/2)*sizeof(pvm_float);

	// what if the database is smaller than UNMEGA
	if (SzForAll<SzPerBlock) return MaxNr;

	// the searched element condition
	if (SzForMid > SzPerBlock-SizePerLine && SzForMid <= SzPerBlock) return MidNr;

	// condition to go left
	if (SzForMid > SzPerBlock) return GetNrRecPerBlock(MinNr, MidNr);

	return GetNrRecPerBlock(MidNr, MaxNr);
}

PreCache::~PreCache()
{

}

bool PreCache::SetParentAlg(GML::Algorithm::IMLAlgorithm* _alg)
{
	this->alg = _alg;
	return true;
}

bool PreCache::ThreadPrecomputeBlock(GML::Algorithm::MLThreadData &thData)
{
	int ThreadId = thData.ThreadID, OutPos, LineNr;
	GML::ML::MLRecord one, two;
	pvm_float sum;

	CHECKMSG(id.con->CreateMlRecord(one),"could not create MLRecord");
	CHECKMSG(id.con->CreateMlRecord(two),"could not create MLRecord");

	if (ThreadId == 0) id.notif->StartProcent("[%s] -> Computing Block number %04d ... ",ObjectName, pccb.CurrBlockNr);	

	ker_f_wrapper kernel(con, notif);	
	CHECKMSG(kernel.set_params(id.varKernelParamDouble, id.varKernelParamInt, NULL, (KerFuncType)id.varKernelType),"could not set kernel parameters");
	
	// the outer loop where every thread alternated and takes a record
	for (UInt32 i=pccb.RecStart+ThreadId; i<pccb.RecStart+pccb.RecCount; i+=alg->threadsCount) {

		LineNr = i-pccb.RecStart;

		// get ml record from connector
		CHECKMSG(id.con->GetRecord(one, i),"could not get ml record from connector");

		// iterate through all records
		for (UInt32 j=LineNr;j<NrRec;j++) {

			// get second ml record here
			CHECKMSG(id.con->GetRecord(two, j),"could not get ml record from connector");
			
			// the kernel computation
			sum = (pvm_float)kernel.compute_for(one, two);

			// put the kernel value to its out place			
			OutPos = (LineNr*NrRec - LineNr*(LineNr-1)/2) + j-LineNr;
			pccb.KernelBuffer[OutPos] = sum;			
		}

		// compute the kprime values for this Block			
		pvm_float KpPos = 0, KpNeg = 0, val;

		double label;
		for (UInt32 j=0;j<NrRec;j++) {
			CHECKMSG(GetKernelAt(LineNr,j, pccb.KernelBuffer, pccb.RecCount, &val),"could not get kernel value");		
			CHECKMSG(id.con->GetRecordLabel(label, j),"could not record label for index %d", j);		
			if (label==1) KpPos += val;						
			else KpNeg += val;		
		}

		//todo: we have to divide by sub of weights, now divide by nr of records
		pccb.KPrimeBuffer[LineNr].pos = KpPos/NrPosRec;
		pccb.KPrimeBuffer[LineNr].neg = KpNeg/NrNegRec;

		if (ThreadId == 0 && (i-pccb.RecStart)%10==0) id.notif->SetProcent(i-pccb.RecStart, pccb.RecCount);
	}

	// end the connector procent progress
	id.notif->EndProcent();

	return true;
}

int PreCache::GetSizeOfBlock(int BlockNr)
{
	int RecHere = 0;
	if (BlockNr == TotalNrBlocks-1) 
		RecHere = NrRec - BlockNr * RecPerBlock;
	else
		RecHere = RecPerBlock;

	return (RecHere*NrRec - (RecHere*(RecHere-1))/2)*sizeof(pvm_float);
}

bool PreCache::GetKernelAt(UInt32 line,UInt32 row, pvm_float* KernelStorage,UInt32 NrRecInBlock, pvm_float *KVal)
{
	CHECKMSG(line<NrRecInBlock, "GetKernelAt line nr out of bounds");
	CHECKMSG(row<NrRec,"GetKernelAt row out of bounds");
	if (line>row) { int aux = line; line = row; row=aux; }
	
	int OutPos = (line*NrRec - line*(line-1)/2) + row-line;
	*KVal = KernelStorage[OutPos];
	return true;
}

DWORD PreCache::AtLoadNextBlock()
{
	GML::Utils::File fileObj;
	GML::Utils::GString atBlockFileName;

	UInt64	readUntilNow, readNow, readSz;
	pvm_float* iterBuf;
	PreCacheFileHeader blockHeader, normHeader;

	// looping until somebody requests to terminate by setting KillThread = true
	while (!AtKillThread) {
		// waiting until signaled to work
		ATCHECKMSG(WaitForSingleObject(AtEventWaiting,INFINITE)==WAIT_OBJECT_0,"error waiting at event AtEventWaiting");
		// reset event stating that we are working
		ResetEvent(AtEventWorking);

		//
		// read the block file from disk
		// 

		CHECKMSG(GetBlockFileName(AtBlockId, FileTypeKernel, atBlockFileName), "could not compose block file name");

		// read header
		ATCHECKMSG(fileObj.OpenRead(atBlockFileName.GetText(), true), "could not load file: %s", atBlockFileName.GetText());
		ATCHECKMSG(fileObj.Read(&blockHeader, sizeof(blockHeader),&readNow), "could not read from file: %s", atBlockFileName.GetText());		

		// checks
		ATCHECKMSG(readNow==sizeof(blockHeader), "could not read from file: %s", atBlockFileName.GetText());
		ATCHECKMSG(strcmp(blockHeader.Magic,PRECACHE_FILE_HEADER_MAGIC)==0,"could not verify file magic header for file: %s", atBlockFileName.GetText());		
		ATCHECKMSG(blockHeader.BlockSize<=id.varBlockFileSize*UNMEGA, "the block size in file: %s exceeds declared size", atBlockFileName.GetText());

		// reading file from disk
		readUntilNow = 0;		
		iterBuf = blockHandle[AtIdxLoading].KERN;
		while (readUntilNow<blockHeader.BlockSize) {
			if (blockHeader.BlockSize-readUntilNow<UNMEGA) readSz = blockHeader.BlockSize-readUntilNow;
			else readSz = UNMEGA;

			ATCHECKMSG(fileObj.Read(iterBuf, readSz,&readNow), "could not read from file: %s", atBlockFileName.GetText());		
			ATCHECKMSG(readNow==readSz, "could not enough read from file: %s", atBlockFileName.GetText());
			
			readUntilNow += readNow;
			iterBuf += readSz;
		}
		fileObj.Close();

		//
		// read the eqnorm file from disk
		// 

		// alloc memory for norm buf because we needed to know the record number per block
		if (blockHandle[0].NORM==NULL) {
			for (int i=0;i<PRECACHE_NR_WORK_BUFFERS;i++) {
				blockHandle[i].NORM = (pvm_float*) pvm_malloc(sizeof(pvm_float)*blockHeader.NrRecords);
				NULLCHECKMSG(blockHandle[i].NORM, "could not alloc memory for eq norm buffer");
			}				
		} 

		CHECKMSG(GetBlockFileName(AtBlockId, FileTypeNorm, atBlockFileName), "could not compose block file name");

		// read header
		ATCHECKMSG(fileObj.OpenRead(atBlockFileName.GetText(), true), "could not load file: %s", atBlockFileName.GetText());
		ATCHECKMSG(fileObj.Read(&normHeader, sizeof(normHeader),&readNow), "could not read from file: %s", atBlockFileName.GetText());		

		// checks
		ATCHECKMSG(readNow==sizeof(normHeader), "could not read from file: %s", atBlockFileName.GetText());
		ATCHECKMSG(strcmp(normHeader.Magic,EQNORM_FILE_HEADER_MAGIC)==0,"could not verify file header magic for file: %s", atBlockFileName.GetText());		

		// the data read
		ATCHECKMSG(fileObj.Read(blockHandle[AtIdxLoading].NORM, sizeof(pvm_float)*blockHeader.NrRecords,&readNow), "could not read from file: %s", atBlockFileName.GetText());		
		ATCHECKMSG(readNow==sizeof(pvm_float)*blockHeader.NrRecords, "could not read from file: %s", atBlockFileName.GetText());

		// fill in the other fields of the handle and return
		blockHandle[AtIdxLoading].blkNr = AtBlockId;
		blockHandle[AtIdxLoading].recCount = blockHeader.NrRecords;
		blockHandle[AtIdxLoading].recStart = blockHeader.RecordStart;

		// reset events 
		ResetEvent(AtEventWaiting);
		SetEvent(AtEventWorking);
	}	
	return 0;
}

DWORD WINAPI PreCache::ThreadWrapper(LPVOID args)
{
	PreCache* MyObject = (PreCache*)args;			
	return MyObject->AtLoadNextBlock();	
}

bool PreCache::AtInitLoading()
{
	INFOMSG("Init thread and data for asynchronous block loading");

	ResetEvent(AtEventWaiting);
	SetEvent(AtEventWorking);

	// set all data to zero
	memset((void*)blockHandle, 0, sizeof(BlockLoadHandle)*PRECACHE_NR_WORK_BUFFERS);

	// create the worker thread responsible for block loading
	HANDLE hThread = CreateThread(NULL, 0, &PreCache::ThreadWrapper, this, 0, NULL);
	CHECKMSG(hThread!=INVALID_HANDLE_VALUE, "could not create thread");

	for (int i=0;i<PRECACHE_NR_WORK_BUFFERS;i++) 
	{
		// alloc memory 
		blockHandle[i].KERN = (pvm_float*) pvm_malloc(id.varBlockFileSize*UNMEGA);
		CHECKMSG(blockHandle[i].KERN!=NULL, "could not allocate enough memory");	

		// memory for blockHandle[i].N - eqnorm will be alloc on first encounter
	}
	
	return true;
}

bool PreCache::AtSignalStartLoading(UInt32 blockId)
{
	INFOMSG("Signaled loading of block: %d", blockId);

	// determine which buffer to use now
	if (blockId%2==0) {AtIdxLoading=0; AtIdxExecuting=1;} 
	else {AtIdxLoading=1; AtIdxExecuting=0;}

	// which block to load now?
	AtBlockId = blockId;

	// signal thread to start working
	SetEvent(AtEventWaiting);
	return true;	
}

PreCache::BlockLoadHandle* PreCache::AtWaitForCompletion()
{
	INFOMSG("Waiting for block: %d to complete the loading process", AtBlockId);

	// sleep for sync reasons
	Sleep(100);

	// wait for thread to finish
	CHECKMSG(WaitForSingleObject(AtEventWorking, INFINITE)==WAIT_OBJECT_0,"thread waiting error");
	
	if (AtKillThread) {
		INFOMSG("There was an error somewhere, somebody signaled the thread to finish; returning null");
		return NULL;
	}

	// return the handle 
	return &blockHandle[AtIdxLoading];
}

bool PreCache::GetBlockFileName(UInt32 BlockId, FileType fType, GML::Utils::GString &BlockFileName)
{
	BlockFileName.Truncate(0);
	CHECKMSG(BlockFileName.Add(id.varBlockFilePrefix),"could not add prefix file name");

	switch (fType) {
	case FileTypeKernel:
		CHECKMSG(BlockFileName.AddFormated(".kernel.%02d", BlockId),"could not add .kernel.NR to file name");
		return true;
	case FileTypeKPrime:
		CHECKMSG(BlockFileName.AddFormated(".kprime.%02d", BlockId),"could not add .kprime.NR to file name");
		return true;
	case FileTypeNorm:
		CHECKMSG(BlockFileName.AddFormated(".eqnorm.%02d", BlockId),"could not add .eqnorm.NR to file name");
		return true;
	default:
		ERRORMSG(false, "please give me a valid file type");
		return false;
	}

	return false;
}

bool PreCache::MergeKPrimeFiles() 
{
	unsigned char* buffer,*iterBuf;
	GML::Utils::File blockFileObj, outFileObj;
	GML::Utils::GString blockFileName, outFileName;
	PreCacheFileHeader pcfh;
	PreCacheFileHeader finalPcfh;

	UInt64	readNow, written, totalSize;

	// allocate buffer
	buffer = (unsigned char*)pvm_malloc(NrRec*sizeof(KPrimePair));
	CHECKMSG(buffer, "could not alloc enough memory for merged files");

	outFileName.Truncate(0);
	outFileName.Add(id.varBlockFilePrefix);
	outFileName.Add(".kprime.all");
	CHECKMSG(outFileObj.Create(outFileName.GetText()), "could not open file output file for writing");

	// set finalPcfh header fields
	finalPcfh.NrRecords = NrRec;
	finalPcfh.NrRecordsTotal = NrRec;
	finalPcfh.NrFeatures = NrFts;
	finalPcfh.RecordStart = 0;	
	finalPcfh.BlockSize = 0; // to be added to this value later

	SizePerLine = sizeof(pvm_float)*NrRec;
	//RecPerBlock = GetNrRecPerBlock(0, NrRec);
	RecPerBlock = GetNrRecPerBlockNonRecursive(NrRec);

	TotalNrBlocks = NrRec/RecPerBlock;
	
	if (NrRec%RecPerBlock)
		TotalNrBlocks++;
		
	memset(finalPcfh.Magic, 0, PRECACHE_FILE_HEADER_MAGIC_SZ);
	sprintf_s((char*)finalPcfh.Magic,strlen(KPRIME_FILE_HEADER_MAGIC)+1,KPRIME_FILE_HEADER_MAGIC);

	iterBuf = buffer;
	totalSize = 0;
	//for (UInt32 i=id.varBlockStart;i<id.varBlockStart+id.varBlockCount;i++) {
	for (UInt32 i = 0; i < TotalNrBlocks; i++) //!!!we must have ALL the blocks in order for this to succeed
	{
		
		CHECKMSG(GetBlockFileName(i, FileTypeKPrime, blockFileName),"could not compose block file name");
		INFOMSG("reading from file: %s", blockFileName.GetText());

		// read header
		CHECKMSG(blockFileObj.OpenRead(blockFileName.GetText(), true), "could not load file: %s", blockFileName.GetText());
		CHECKMSG(blockFileObj.Read(&pcfh, sizeof(pcfh),&readNow), "could not read from file: %s", blockFileName.GetText());	

		// checks
		CHECKMSG(readNow==sizeof(pcfh), "could not read from file: %s", blockFileName.GetText());
		CHECKMSG(strcmp(pcfh.Magic,KPRIME_FILE_HEADER_MAGIC)==0,"could not verify file magic header for file: %s", blockFileName.GetText());		
		
		CHECKMSG(blockFileObj.Read(iterBuf, pcfh.BlockSize, &readNow),"error reading from file");
		CHECKMSG(pcfh.BlockSize==readNow, "could not read enough from file");

		iterBuf += pcfh.BlockSize;
		totalSize += pcfh.BlockSize;
	}

	finalPcfh.BlockSize = totalSize;

	// write the header
	CHECKMSG(outFileObj.Write(&finalPcfh, sizeof(finalPcfh), &written), "error writing to file");
	CHECKMSG(sizeof(finalPcfh)==written, "could not write enough to output file");

	// write the contents
	CHECKMSG(outFileObj.Write(buffer, totalSize, &written), "error writing to file");
	CHECKMSG(totalSize==written, "could not write enough to output file");
	outFileObj.Close();	

	return true;
}

bool PreCache::PreComputeNorm()
{
	GML::Utils::GString kprimeFn;
	GML::Utils::File    kprimeFo;

	GML::Utils::GString blockFn;
	GML::Utils::File    blockFo;

	GML::Utils::GString normFn;
	GML::Utils::File    normFo;
	
	PreCacheFileHeader kpFh;
	PreCacheFileHeader blockFh;
	PreCacheFileHeader normFh;

	UInt64	read, readUnow, readSz, written;
	pvm_float  *blkBuf, *blkBufIt, *normBuf;
	KPrimePair *kpBuf;

	double  recLabel;
	pvm_float sum, tsum, kval;
	

	INFOMSG("Loading kprime.all file");

	kprimeFn.Truncate(0);
	kprimeFn.Add(id.varBlockFilePrefix);
	kprimeFn.Add(".kprime.all");

	// read kprime file header to establish the size of the buffer we need to alloc here
	CHECKMSG(kprimeFo.OpenRead(kprimeFn),"could not open %s for reading", kprimeFn.GetText());
	CHECKMSG(kprimeFo.Read(&kpFh, sizeof(kpFh),&read), "could not read from file");
	CHECKMSG(sizeof(kpFh)==read, "could not read enough");

	// alloc memory for kpBuf
	kpBuf = (KPrimePair*) pvm_malloc((UInt32)kpFh.BlockSize);
	NULLCHECKMSG(kpBuf, "could not alloc enough memory");

	CHECKMSG(kprimeFo.Read(kpBuf, kpFh.BlockSize, &read),"could not read from file");
	CHECKMSG(kpFh.BlockSize==read, "could not read enough");
	
	/*kpBuf = (KPrimePair*) pvm_malloc((UInt32)kpFh.NrRecordsTotal * sizeof(KPrimePair));
	NULLCHECKMSG(kpBuf, "could not alloc enough memory");

	CHECKMSG(kprimeFo.Read(kpBuf, (UInt32)kpFh.NrRecordsTotal * sizeof(KPrimePair), &read),"could not read from file");
	CHECKMSG((UInt32)kpFh.NrRecordsTotal * sizeof(KPrimePair)==read, "could not read enough");
	  */
	blkBuf = normBuf =NULL;

	// loop through the block files that we need to process and compute the norms
	for (UInt32 blockIdx = id.varBlockStart; blockIdx < id.varBlockStart+id.varBlockCount; blockIdx ++) 
	{		
		CHECKMSG(GetBlockFileName(blockIdx, FileTypeKernel, blockFn), "could not make block file name");
		
		CHECKMSG(blockFo.OpenRead(blockFn),"could not open file: %s",blockFn.GetText());
		CHECKMSG(blockFo.Read(&blockFh, sizeof(blockFh), &read), "could not read from file");
		CHECKMSG(sizeof(blockFh)==read, "could not read enough");

		// we need to alloc mem now?
		if (blkBuf==NULL || normBuf==NULL) {
			blkBuf = (pvm_float*) pvm_malloc((UInt32)blockFh.BlockSize);
			NULLCHECKMSG(blkBuf, "could not alloc enough memory");

			// alloc memory for norms
			normBuf = (pvm_float*) pvm_malloc(blockFh.NrRecords*sizeof(pvm_float));
			NULLCHECKMSG(normBuf, "could not alloc enough memory");
		}

		// read the block file
		readUnow = 0;
		blkBufIt = blkBuf;
		while (readUnow < blockFh.BlockSize) {
			readSz = UNMEGA;
			if (blockFh.BlockSize-readUnow < UNMEGA) readSz = blockFh.BlockSize-readUnow;
			
			CHECKMSG(blockFo.Read(blkBufIt, readSz, &read), "could not read from file");
			CHECKMSG(readSz==read, "could not read enough");

			readUnow += read;
		}

		// compute the norm of the equations associated with this block
		for (UInt32 blkRecIdx = 0; blkRecIdx < blockFh.NrRecords; blkRecIdx++)
		{
			CHECKMSG(con->GetRecordLabel(recLabel, blkRecIdx+blockFh.RecordStart), "could not get record label");
			tsum = 1;
			sum  = 0;
			for (UInt32 recIdx=0;recIdx<kpFh.NrRecordsTotal;recIdx++) 
			{
				CHECKMSG(GetKernelAt(blkRecIdx, recIdx, blkBuf, blockFh.NrRecords, &kval),"could not get kernel value");

				sum = kval - ((recLabel==1) ?kpBuf[recIdx].pos :kpBuf[recIdx].neg);
				tsum += sum*sum;
			}
			normBuf[blkRecIdx] = sqrt(tsum);
		}

		// make the eq norm file header
		normFh.BlockSize = blockFh.NrRecords*sizeof(pvm_float);
		normFh.NrFeatures = blockFh.NrFeatures;
		normFh.NrRecords  = blockFh.NrRecords;
		normFh.NrRecordsTotal = blockFh.NrRecordsTotal;
		normFh.RecordStart = blockFh.RecordStart;

		// set the file header fields
		memset(normFh.Magic, 0, PRECACHE_FILE_HEADER_MAGIC_SZ);		
		sprintf_s((char*)normFh.Magic,strlen(EQNORM_FILE_HEADER_MAGIC)+1,EQNORM_FILE_HEADER_MAGIC);	
		
		// create output file
		CHECKMSG(GetBlockFileName(blockIdx, FileTypeNorm, normFn), "could not make block file name");
		CHECKMSG(normFo.Create(normFn.GetText()),"could not create output file");		

		// write file header				
		CHECKMSG(normFo.Write(&normFh, sizeof(normFh),&written),"could not write to file");
		CHECKMSG(sizeof(normFh)==written, "could not read enough");

		CHECKMSG(normFo.Write(normBuf, normFh.BlockSize,&written),"could not write to file");
		CHECKMSG(normFh.BlockSize==written, "could not read enough");
		normFo.Close();
	}

	return true;
}
