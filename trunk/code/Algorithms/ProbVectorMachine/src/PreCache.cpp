
#include "PreCache.h"
#include "ProbVectorMachine.h"

PreCache::PreCache()
{
	this->ObjectName = "ProbVectorMachine";	
}

bool PreCache::SetInheritData(PreCache::InheritData &InhData)
{
	this->id = InhData;
	this->con = id.con;
	this->notif = id.notif;
	return true;
}

bool PreCache::PreCompute()
{
	GML::Utils::File KernelFileObj, KPrimeFileObj;
	GML::Utils::GString KernelFileName, KPrimeFileName; 
	unsigned char *KernelBuffer, *WriteBuffer;
	KPrimePair  *KPrimeBuffer;
	unsigned long long SzOfBatch, WrittenSoFar, Written, BlockSize;	
	PreCacheFileHeader pcfh;

	NrRec = id.con->GetRecordCount();
	NrFts = id.con->GetFeatureCount();

	SizePerLine = sizeof(pvm_float)*NrRec;
	RecPerBatch = GetNrRecPerBatch(0, NrRec);
	TotalNrBatches = NrRec/RecPerBatch+1;

	if (id.VarPreCacheBatchCount>=TotalNrBatches)
		id.VarPreCacheBatchCount = TotalNrBatches;

	// allocate memory for output buffer
	KernelBuffer = (unsigned char*) malloc (id.VarPreCacheFileSize*UNMEGA);
	NULLCHECKMSG(KernelBuffer, "could not allocate enough memory for kernel output buffer");
	pccb.KernelBuffer = (pvm_float*) KernelBuffer;
	memset(KernelBuffer, 0, id.VarPreCacheFileSize*UNMEGA);

	KPrimeBuffer = (KPrimePair*) malloc (RecPerBatch*sizeof(KPrimePair));
	NULLCHECKMSG(KPrimeBuffer, "could not allocate enough memory for kprime output buffer");
	pccb.KPrimeBuffer = KPrimeBuffer;
	memset(KPrimeBuffer, 0, RecPerBatch*sizeof(KPrimePair));

	// records per batch, useful for computing where we are right now
	pccb.RecPerBatch = RecPerBatch;

	// iterate through the batches that need to be computed
	for (UInt32 i=id.VarPreCacheBatchStart;i<id.VarPreCacheBatchStart+id.VarPreCacheBatchCount && i*RecPerBatch<NrRec;i++) {
		
		// create the current output file
		KernelFileName.Truncate(0);
		KernelFileName.Add(id.VarPreCacheFilePrefix);
		KernelFileName.AddFormated(".kernel.%02d", i);		

		KPrimeFileName.Truncate(0);
		KPrimeFileName.Add(id.VarPreCacheFilePrefix);
		KPrimeFileName.AddFormated(".kprime.%02d", i);

		CHECKMSG(KernelFileObj.Create(KernelFileName), "could not create file: %s", KernelFileName.GetText());
		CHECKMSG(KPrimeFileObj.Create(KPrimeFileName), "could not create file: %s", KPrimeFileName.GetText());

		// set the current batch number for the map operation
		pccb.CurrBatchNr = i;
		pccb.RecStart = i*RecPerBatch;
		if (i==TotalNrBatches-1) pccb.RecCount = NrRec - i*RecPerBatch;
		else pccb.RecCount = RecPerBatch;
					  
		// run the actual map command on threads
		alg->ExecuteParalelCommand(ProbVectorMachine::THREAD_COMMAND_PRECOMPUTE_BATCH);

		//
		// write the out buffer to the disk
		// 
		SzOfBatch = GetSizeOfBatch(i);
		WriteBuffer = KernelBuffer;

		// set the file header fields
		memset(pcfh.Magic, 0, 32);		
		sprintf_s((char*)pcfh.Magic,strlen(PRECACHE_FILE_HEADER_MAGIC)+1,PRECACHE_FILE_HEADER_MAGIC);		

		pcfh.NrFeatures = NrFts;
		pcfh.NrRecordsTotal = NrRec;
		pcfh.NrRecords = pccb.RecCount;
		pcfh.RecordStart = i*RecPerBatch;		

		//write the header of the PreCache file
		DBG_CHECKMSG(KernelFileObj.Write(0, &pcfh, sizeof(PreCacheFileHeader), &Written), "failed to write kernel values file header");

		// the write loop
		WrittenSoFar = Written;
		while (WrittenSoFar < SzOfBatch+sizeof(PreCacheFileHeader)) {			
			// figure how much to write to this block
			//if (SzOfBatch-WrittenSoFar < UNMEGA) BlockSize = SzOfBatch-WrittenSoFar;
			//else BlockSize = UNMEGA;
		
			// the write operation
			DBG_CHECKMSG(KernelFileObj.Write(WrittenSoFar, WriteBuffer, UNMEGA, &Written), "failed to write kernel values buffer");
			WrittenSoFar += Written;
		}

		memset(pcfh.Magic, 0, 32);
		sprintf_s((char*)pcfh.Magic,strlen(KPRIME_FILE_HEADER_MAGIC)+1,KPRIME_FILE_HEADER_MAGIC);
		//write the header of the PreCache file
		DBG_CHECKMSG(KPrimeFileObj.Write(0, &pcfh, sizeof(PreCacheFileHeader), &Written), "failed to write kprime values file header");
		// write kprim buffer
		DBG_CHECKMSG(KPrimeFileObj.Write(Written, KPrimeBuffer, pccb.RecCount*sizeof(KPrimePair), &Written), "failed to write kprime buffer");

		KernelFileObj.Close();
		KPrimeFileObj.Close();
		pccb.BatchNr ++;
	}

	// free the buffers
	free(KernelBuffer);
	free(KPrimeBuffer);

	return true;
}

int PreCache::GetNrRecPerBatch(int MinNr, int MaxNr)
{
	// binary search for the nr of records per batch
	UInt32 MidNr = MinNr + (MaxNr-MinNr)/2;
	UInt32 SzForMid = (MidNr*NrRec - (MidNr*(MidNr-1))/2)*sizeof(pvm_float);
	UInt32 SzPerBatch = id.VarPreCacheFileSize*UNMEGA;
	UInt32 SzForAll = (NrRec*NrRec - (NrRec*(NrRec-1))/2)*sizeof(pvm_float);

	// what if we the database is smaller than UNMEGA
	if (SzForAll<SzPerBatch) return MaxNr;

	// the searched element condition
	if (SzForMid > SzPerBatch-SizePerLine && SzForMid <= SzPerBatch) return MidNr;

	// condition to go left
	if (SzForMid > SzPerBatch) return GetNrRecPerBatch(MinNr, MidNr);

	return GetNrRecPerBatch(MidNr, MaxNr);
}

PreCache::~PreCache()
{

}

bool PreCache::SetParentAlg(GML::Algorithm::IMLAlgorithm* _alg)
{
	this->alg = _alg;
	return true;
}

bool PreCache::ThreadPrecomputeBatch(GML::Algorithm::MLThreadData &thData)
{
	int ThreadId = thData.ThreadID, OutPos, LineNr;
	GML::ML::MLRecord one, two;
	pvm_float sum;

	DBG_CHECKMSG(id.con->CreateMlRecord(one),"could not create MLRecord");
	DBG_CHECKMSG(id.con->CreateMlRecord(two),"could not create MLRecord");

	if (ThreadId == 0) id.notif->StartProcent("[%s] -> Computing batch number %03d ... ",ObjectName, pccb.CurrBatchNr);	

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

		// compute the kprime values for this batch			
		pvm_float KpPos = 0, KpNeg = 0, val;

		double label;
		for (UInt32 j=0;j<NrRec;j++) {
			CHECK(GetKernelAt(LineNr,j, pccb.KernelBuffer, pccb.RecCount, &val));		
			CHECKMSG(id.con->GetRecordLabel(label, j),"could not record label for index %d", j);		
			if (label==1) KpPos += val;						
			else KpNeg += val;		
		}
		pccb.KPrimeBuffer[LineNr].pos = KpPos;
		pccb.KPrimeBuffer[LineNr].neg = KpNeg;

		if (ThreadId == 0 && (i-pccb.RecStart)%10==0) id.notif->SetProcent(i-pccb.RecStart, pccb.RecCount);
	}

	// end the connector procent progress
	id.notif->EndProcent();

	return true;
}

int PreCache::GetSizeOfBatch(int BatchNr)
{
	int RecHere = 0;
	if (BatchNr == TotalNrBatches-1) 
		RecHere = NrRec - BatchNr * RecPerBatch;
	else
		RecHere = RecPerBatch;

	return (RecHere*NrRec - (RecHere*(RecHere-1))/2)*sizeof(pvm_float);
}

bool PreCache::GetKernelAt(int line, int row, pvm_float* KernelStorage, int NrRecInBatch, pvm_float *KVal)
{
	DBGSTOP_CHECKMSG(line<NrRecInBatch, "GetKernelAt line nr out of bounds");
	DBGSTOP_CHECKMSG(row<NrRec,"GetKernelAt row out of bounds");
	if (line>row) { int aux = line; line = row; row=aux; }
	
	int OutPos = (line*NrRec - line*(line-1)/2) + row-line;
	*KVal = KernelStorage[OutPos];
	return true;
}
