
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
	GML::Utils::File BatchFileObj;
	GML::Utils::GString BatchFileName; 
	unsigned char *Buffer, *WriteBuffer;
	unsigned long long SzOfBatch, WrittenSoFar, Written, BlockSize;

	NrRec = id.con->GetRecordCount();
	NrFts = id.con->GetFeatureCount();

	SizePerLine = sizeof(pvm_float)*NrRec;
	RecPerBatch = GetNrRecPerBatch(0, NrRec);
	TotalNrBatches = NrRec/RecPerBatch+1;

	if (id.VarPreCacheBatchCount>=TotalNrBatches)
		id.VarPreCacheBatchCount = TotalNrBatches;

	// allocate memory for output buffer
	Buffer = (unsigned char*) malloc (id.VarPreCacheFileSize*UNMEGA);
	NULLCHECKMSG(Buffer, "could not allocate enough memory for output buffer");
	pccb.Buffer = (pvm_float*) Buffer;

	// iterate through the batches that need to be computed
	for (UInt32 i=id.VarPreCacheBatchStart;i<id.VarPreCacheBatchStart+id.VarPreCacheBatchCount;i++) {
		
		// create the current output file
		BatchFileName.Truncate(0);
		BatchFileName.Add(id.VarPreCacheFilePattern);
		BatchFileName.AddFormated("%03d", i);
		CHECKMSG(BatchFileObj.Create(BatchFileName), "could not create file: %s", BatchFileName.GetText());

		// set the current batch number for the map operation
		pccb.CurrBatchNr = i;
		pccb.RecStart = i*RecPerBatch;
		if (i==TotalNrBatches-1) pccb.RecCount = NrRec - i*RecPerBatch;
		else pccb.RecCount = RecPerBatch;
					  
		// run the actual map command on threads
		alg->ExecuteParalelCommand(ProbVectorMachine::THREAD_COMMAND_PRECOMPUTE_BATCH);

		// write the out buffer to the disk
		SzOfBatch = GetSizeOfBatch(i);
		WriteBuffer = Buffer;

		// the write loop
		WrittenSoFar = 0;
		while (WrittenSoFar < SzOfBatch) {			
			// figure how much to write to this block
			if (SzOfBatch-WrittenSoFar < UNMEGA) BlockSize = SzOfBatch-WrittenSoFar;
			else BlockSize = UNMEGA;
		
			// the write operation
			DBG_CHECKMSG(BatchFileObj.Write(WrittenSoFar, WriteBuffer, UNMEGA, &Written), "failed to write on disk");
			WrittenSoFar += Written;
		}
		BatchFileObj.Close();
	}

	return true;
}

int PreCache::GetNrRecPerBatch(int MinNr, int MaxNr)
{
	// binary search for the nr of records per batch
	unsigned int MidNr = MinNr + (MaxNr-MinNr)/2;
	unsigned int SzForMid = (MidNr*NrRec - (MidNr*(MidNr-1))/2)*sizeof(pvm_float);
	unsigned int SzPerBatch = id.VarPreCacheFileSize*UNMEGA;

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
	int ThreadId = thData.ThreadID, OutPos, NrLines;
	GML::ML::MLRecord one, two;
	pvm_float sum;

	DBG_CHECKMSG(id.con->CreateMlRecord(one),"could not create MLRecord");
	DBG_CHECKMSG(id.con->CreateMlRecord(two),"could not create MLRecord");

	if (ThreadId == 0) id.notif->StartProcent("[%s] -> Computing batch number %03d ... ",ObjectName, pccb.CurrBatchNr);	
	
	// the outer loop where every thread alternated and takes a record
	for (UInt32 i=pccb.RecStart+ThreadId; i<pccb.RecStart+pccb.RecCount; i+=alg->threadsCount) {

		NrLines = i-pccb.RecStart;

		// get ml record from connector
		DBG_CHECKMSG(id.con->GetRecord(one, i),"could not get ml record from connector");

		// iterate through all records
		for (UInt32 j=NrLines;j<NrRec;j++) {

			// get second ml record here
			DBG_CHECKMSG(id.con->GetRecord(two, j),"could not get ml record from connector");
			
			// insert kernel here; placeholder now
			sum = 0;
			for (UInt32 k=0;k<NrFts;k++) 
				sum += (pvm_float)((one.Features[k]-two.Features[k])*(one.Features[k]-two.Features[k]));

			// put the kernel value to its out place			
			OutPos = (NrLines*NrRec - NrLines*(NrLines-1)/2) + j-NrLines;
			pccb.Buffer[OutPos] = sum;			
		}
		if (ThreadId == 0 && i%10==0) id.notif->SetProcent(i-pccb.RecStart, pccb.RecCount);
	}
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
