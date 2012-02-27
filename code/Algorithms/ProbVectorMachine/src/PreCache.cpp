
#include "PreCache.h"
#include "ProbVectorMachine.h"

PreCache::PreCache()
{
	this->ObjectName = "ProbVectorMachine";	

	AtKillThread = false;
	AtEventWorking = CreateEvent(NULL,TRUE,FALSE,NULL);
	AtEventWaiting = CreateEvent(NULL,TRUE,TRUE,NULL);

}

bool PreCache::SetInheritData(PreCache::InheritData &InhData)
{
	this->id = InhData;
	this->con = id.con;
	this->notif = id.notif;

	NrRec = id.con->GetRecordCount();
	NrFts = id.con->GetFeatureCount();

	return true;
}

bool PreCache::PreCompute()
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
	for (int i=0;i<NrRec;i++) {
		CHECKMSG(con->GetRecordLabel(label, i), "error getting record label: %d", i);
		if (label == 1) NrPosRec++;
		else NrNegRec++;
	}

	SizePerLine = sizeof(pvm_float)*NrRec;
	RecPerBlock = GetNrRecPerBlock(0, NrRec);
	TotalNrBlockes = NrRec/RecPerBlock+1;

	if (id.VarPreCacheBlockCount>=TotalNrBlockes)
		id.VarPreCacheBlockCount = TotalNrBlockes;

	// allocate memory for output buffer
	kernelBuffer = (unsigned char*) malloc (id.VarPreCacheFileSize*UNMEGA);
	NULLCHECKMSG(kernelBuffer, "could not allocate enough memory for kernel output buffer");
	pccb.KernelBuffer = (pvm_float*) kernelBuffer;
	memset(kernelBuffer, 0, id.VarPreCacheFileSize*UNMEGA);

	kprimeBuffer = (KPrimePair*) malloc (RecPerBlock*sizeof(KPrimePair));
	NULLCHECKMSG(kprimeBuffer, "could not allocate enough memory for kprime output buffer");
	pccb.KPrimeBuffer = kprimeBuffer;
	memset(kprimeBuffer, 0, RecPerBlock*sizeof(KPrimePair));

	// records per Block, useful for computing where we are right now
	pccb.RecPerBlock = RecPerBlock;

	// iterate through the Blockes that need to be computed
	for (UInt32 i=id.VarPreCacheBlockStart;i<id.VarPreCacheBlockStart+id.VarPreCacheBlockCount && i*RecPerBlock<NrRec;i++) {
		
		// create the current output file
		CHECKMSG(GetBlockFileName(i, true, kernelFileName),"could not compose kernel file name");		
		CHECKMSG(GetBlockFileName(i, false, kprimeFileName),"could not compose kprime file name");		

		CHECKMSG(kernelFileObj.Create(kernelFileName), "could not create file: %s", kernelFileName.GetText());
		CHECKMSG(kprimeFileObj.Create(kprimeFileName), "could not create file: %s", kprimeFileName.GetText());

		// set the current Block number for the map operation
		pccb.CurrBlockNr = i;
		pccb.RecStart = i*RecPerBlock;
		if (i==TotalNrBlockes-1) pccb.RecCount = NrRec - i*RecPerBlock;
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
		DBG_CHECKMSG(kernelFileObj.Write(0, &pcfh, sizeof(PreCacheFileHeader), &written), "failed to write kernel values file header");

		// the write loop
		writtenSoFar = written;
		while (writtenSoFar < sizeOfBlock+sizeof(PreCacheFileHeader)) {			
			// figure how much to write to this block
			//if (SzOfBlock-WrittenSoFar < UNMEGA) BlockSize = SzOfBlock-WrittenSoFar;
			//else BlockSize = UNMEGA;
		
			// the write operation
			DBG_CHECKMSG(kernelFileObj.Write(writtenSoFar, writeBuffer, UNMEGA, &written), "failed to write kernel values buffer");
			writtenSoFar += written;
		}

		// set the magic field
		memset(pcfh.Magic, 0, PRECACHE_FILE_HEADER_MAGIC_SZ);
		sprintf_s((char*)pcfh.Magic,strlen(KPRIME_FILE_HEADER_MAGIC)+1,KPRIME_FILE_HEADER_MAGIC);
		
		// set the block size of the kprime buffer
		pcfh.BlockSize = pccb.RecCount*sizeof(KPrimePair);

		//write the header of the PreCache file
		DBG_CHECKMSG(kprimeFileObj.Write(0, &pcfh, sizeof(PreCacheFileHeader), &written), "failed to write kprime values file header");
		// write kprim buffer
		DBG_CHECKMSG(kprimeFileObj.Write(written, kprimeBuffer, pccb.RecCount*sizeof(KPrimePair), &written), "failed to write kprime buffer");

		kernelFileObj.Close();
		kprimeFileObj.Close();
		pccb.BlockNr ++;
	}

	// free the buffers
	free(kernelBuffer);
	free(kprimeBuffer);

	return true;
}

int PreCache::GetNrRecPerBlock(int MinNr, int MaxNr)
{
	// binary search for the nr of records per Block
	UInt32 MidNr = MinNr + (MaxNr-MinNr)/2;
	UInt32 SzForMid = (MidNr*NrRec - (MidNr*(MidNr-1))/2)*sizeof(pvm_float);
	UInt32 SzPerBlock = id.VarPreCacheFileSize*UNMEGA;
	UInt32 SzForAll = (NrRec*NrRec - (NrRec*(NrRec-1))/2)*sizeof(pvm_float);

	// what if we the database is smaller than UNMEGA
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

	DBG_CHECKMSG(id.con->CreateMlRecord(one),"could not create MLRecord");
	DBG_CHECKMSG(id.con->CreateMlRecord(two),"could not create MLRecord");

	if (ThreadId == 0) id.notif->StartProcent("[%s] -> Computing Block number %03d ... ",ObjectName, pccb.CurrBlockNr);	

	ker_f_wrapper kernel(con, notif);	
	CHECKMSG(kernel.set_params(id.VarKernelParamDouble, id.VarKernelParamInt, NULL, (KerFuncType)id.VarKernelType),"could not set kernel parameters");
	
	// the outer loop where every thread alternated and takes a record
	for (UInt32 i=pccb.RecStart+ThreadId; i<pccb.RecStart+pccb.RecCount; i+=alg->threadsCount) {

		LineNr = i-pccb.RecStart;

		// get ml record from connector
		DBG_CHECKMSG(id.con->GetRecord(one, i),"could not get ml record from connector");

		// iterate through all records
		for (UInt32 j=LineNr;j<NrRec;j++) {

			// get second ml record here
			DBG_CHECKMSG(id.con->GetRecord(two, j),"could not get ml record from connector");
			
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
			CHECK(GetKernelAt(LineNr,j, pccb.KernelBuffer, pccb.RecCount, &val));		
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
	if (BlockNr == TotalNrBlockes-1) 
		RecHere = NrRec - BlockNr * RecPerBlock;
	else
		RecHere = RecPerBlock;

	return (RecHere*NrRec - (RecHere*(RecHere-1))/2)*sizeof(pvm_float);
}

bool PreCache::GetKernelAt(int line, int row, pvm_float* KernelStorage, int NrRecInBlock, pvm_float *KVal)
{
	DBGSTOP_CHECKMSG(line<NrRecInBlock, "GetKernelAt line nr out of bounds");
	DBGSTOP_CHECKMSG(row<NrRec,"GetKernelAt row out of bounds");
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
	unsigned char* iterBuf;
	PreCacheFileHeader pcfh;

	// looping until somebody requests to terminate by setting KillThread = true
	while (!AtKillThread) {
		// waiting until signaled to work
		ATCHECKMSG(WaitForSingleObject(AtEventWaiting,INFINITE)==WAIT_OBJECT_0,"error waiting at event AtEventWaiting");
		// reset event stating that we are working
		ResetEvent(AtEventWorking);

		CHECKMSG(GetBlockFileName(AtBlockId, true, atBlockFileName), "could not compose block file name");
		INFOMSG("loading block file: %s", atBlockFileName.GetText());

		// read header
		ATCHECKMSG(fileObj.OpenRead(atBlockFileName.GetText(), true), "could not load file: %s", atBlockFileName.GetText());
		ATCHECKMSG(fileObj.Read(&pcfh, sizeof(pcfh),&readNow), "could not read from file: %s", atBlockFileName.GetText());		

		// checks
		ATCHECKMSG(readNow==sizeof(pcfh), "could not read from file: %s", atBlockFileName.GetText());
		ATCHECKMSG(strcmp(pcfh.Magic,PRECACHE_FILE_HEADER_MAGIC)==0,"could not verify file magic header for file: %s", atBlockFileName.GetText());
		// todo: check that the size of the block does not exceed the allocated size of the buffer

		// reading file from disk
		readUntilNow = 0;
		iterBuf = AtBuffer[AtIdxLoading];
		while (readUntilNow<pcfh.BlockSize) {
			if (pcfh.BlockSize-readUntilNow<UNMEGA) readSz = pcfh.BlockSize-readUntilNow;
			else readSz = UNMEGA;

			ATCHECKMSG(fileObj.Read(iterBuf, readSz,&readNow), "could not read from file: %s", atBlockFileName.GetText());		
			ATCHECKMSG(readNow==readSz, "could not enough read from file: %s", atBlockFileName.GetText());
			
			readUntilNow += readNow;
			iterBuf += readSz;
		}

		// reset events 
		ResetEvent(AtEventWaiting);
		SetEvent(AtEventWorking);
	}	
}

DWORD WINAPI PreCache::ThreadWrapper(LPVOID args)
{
	PcThreadInfo * pti = (PcThreadInfo*) args;
	switch (pti->FuncId) {
	case LoadBlock: {
		PreCache* MyObject = (PreCache*)pti->Object;			
		return MyObject->AtLoadNextBlock();
		}
	default:
		return 0xffFFffFF;
	}		
}

bool PreCache::TestAtLoading()
{
	PcThreadInfo pti;
	pti.FuncId = LoadBlock;
	pti.Object = this;

	ResetEvent(AtEventWaiting);
	SetEvent(AtEventWorking);

	HANDLE hThread = CreateThread(NULL, 0, &PreCache::ThreadWrapper, &pti, 0, NULL);
	CHECKMSG(hThread!=INVALID_HANDLE_VALUE, "could not create thread");

	AtBuffer[0] = (unsigned char*) malloc(id.VarPreCacheFileSize*UNMEGA);
	CHECKMSG(AtBuffer[0]!=NULL, "could not allocate enough memory");
	AtBuffer[1] = (unsigned char*) malloc(id.VarPreCacheFileSize*UNMEGA);
	CHECKMSG(AtBuffer[1]!=NULL, "could not allocate enough memory");

	for (int i=id.VarPreCacheBlockStart;i<id.VarPreCacheBlockStart+id.VarPreCacheBlockCount;i++) {
		if (i%2==0) {AtIdxLoading=0; AtIdxExecuting=1;} 
		else {AtIdxLoading=1; AtIdxExecuting=0;}
		AtBlockId = i;
		// signal thread to start working
		SetEvent(AtEventWaiting);
		Sleep(100);
		// wait for thread to finish
		CHECKMSG(WaitForSingleObject(AtEventWorking, INFINITE)==WAIT_OBJECT_0,"thread waiting error");
	}

	return true;	
}

bool PreCache::GetBlockFileName (UInt32 BlockId, bool KernelValues, GML::Utils::GString &BlockFileName)
{
	BlockFileName.Truncate(0);
	CHECKMSG(BlockFileName.Add(id.VarPreCacheFilePrefix),"could not add prefix file name");
	if (KernelValues) {
		CHECKMSG(BlockFileName.AddFormated(".kernel.%02d", BlockId),"could not add .kernel.NR to file name");
	} else {
		CHECKMSG(BlockFileName.AddFormated(".kprime.%02d", BlockId),"could not add .kprime.NR to file name");
	}

	return true;
}

bool PreCache::MergeKPrimeFiles() 
{
	unsigned char* buffer,*iterBuf;
	GML::Utils::File blockFileObj, outFileObj;
	GML::Utils::GString blockFileName, outFileName;
	PreCacheFileHeader pcfh;

	UInt64	readNow, written, totalSize;

	// allocate buffer
	buffer = (unsigned char*)malloc(NrRec*sizeof(KPrimePair));
	CHECKMSG(buffer, "could not alloc enough memory for merged files");

	outFileName.Truncate(0);
	outFileName.Add(id.VarPreCacheFilePrefix);
	outFileName.Add(".kprime.all");
	CHECKMSG(outFileObj.Create(outFileName.GetText()), "could not open file output file for writing");

	iterBuf = buffer;
	totalSize = 0;
	for (int i=id.VarPreCacheBlockStart;i<id.VarPreCacheBlockStart+id.VarPreCacheBlockCount;i++) {
		
		CHECKMSG(GetBlockFileName(i, false, blockFileName),"could not compose block file name");
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

	// write to output file
	CHECKMSG(outFileObj.Write(buffer, totalSize, &written), "error writing to file");
	CHECKMSG(totalSize==written, "could not write enough to output file");
	outFileObj.Close();

	return true;
}
