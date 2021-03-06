#include "FullCacheConnector.h"

#define CACHE_SIG_NAME "FullCacheConnectorV3"

FullCacheConnector::FullCacheConnector()
{
	 Records = NULL;
	 Labels  = NULL;	 
	 Weights = NULL;

	 ObjectName = "FullCacheConnector";
	
	AddCacheProperties();
	AddStoreProperties();
}
FullCacheConnector::~FullCacheConnector()
{
	if (Records != NULL) 	
		delete Records;

	if (Labels!= NULL)
		delete Labels;

	if (Weights!= NULL)
		delete Labels;

	Records = NULL;
	Labels = NULL;
}

bool   FullCacheConnector::GetRecord( MLRecord &record,UInt32 index,UInt32 recordMask )
{
	if (index >= nrRecords) 
	{
		notifier->Error("[%s] -> index is out of range, it has to be less than %d", nrRecords);
		return false;
	}
	record.Features = (double*) &Records[index*columns.nrFeatures]; 	
	record.Label = Labels[index];	
	
	return true;
}
bool   FullCacheConnector::CreateMlRecord( MLRecord &record )
{	
	record.FeatCount = columns.nrFeatures;
	return true;
}

bool   FullCacheConnector::OnInitConnectionToDataBase()
{
	UInt32										tr,gr;
	GML::Utils::GTFVector<GML::DB::DBRecord>	VectPtr;
	double										*cPoz;
	double										cValue;
	GML::DB::RecordHash							cHash;
	
	// aloca data :D
	if ((Records = new double [nrRecords*columns.nrFeatures])==NULL)
	{
		notifier->Error("[%s] -> Unable to allocate %ud bytes for data indexes !",ObjectName,nrRecords*columns.nrFeatures*sizeof(double));
		return false;
	}
	if ((Labels = new double[nrRecords])==NULL)
	{
		notifier->Error("[%s] -> Unable to allocate %ud bytes for labels !",ObjectName,nrRecords*sizeof(double));
		return false;
	}
	if ((Weights = new double[nrRecords])==NULL)
	{
		notifier->Error("[%s] -> Unable to allocate %ud bytes for weights !",ObjectName,nrRecords*sizeof(double));
		return false;
	}
	memset(Records,0,nrRecords*columns.nrFeatures*sizeof(double));
	memset(Labels,0,nrRecords*sizeof(double));
	memset(Weights,1,nrRecords*sizeof(double));
	// sunt exact la inceput
	cPoz = Records;

	notifier->StartProcent("[%s] -> Loading DataBase : ",ObjectName);

	for (tr=0;tr<nrRecords;tr++)
	{
		if (database->ReadNextRecord(VectPtr)==false)
		{
			notifier->Error("[%s] -> Unable to read #d record from database!",ObjectName,tr);
			return false;
		}
		// pentru fiecare record pun valorile
		for (gr=0;gr<columns.nrFeatures;gr++)
		{
			if (UpdateDoubleValue(VectPtr,columns.indexFeature[gr],cValue)==false)
				return false;
			cPoz[gr]=cValue;
		}
		// pun si label-ul
		if (UpdateDoubleValue(VectPtr,columns.indexLabel,cValue)==false)
			return false;
		Labels[tr]=cValue;
		// adaug si Hash-ul
		if (StoreRecordHash)
		{
			if (UpdateHashValue(VectPtr,columns.indexHash,cHash)==false)
				return false;
			if (Hashes.PushByRef(cHash)==false)
			{
				notifier->Error("[%s] -> Unable to save Hash with id %d",ObjectName,tr);
				return false;
			}
		}
		// trecem la urmatorul record
		cPoz+=columns.nrFeatures;
		if ((tr % 1000)==0)
			notifier->SetProcent(tr,nrRecords);
	}	
	notifier->EndProcent();
	dataMemorySize = (UInt64)(nrRecords+1) * sizeof(double);

	return true;
}
bool   FullCacheConnector::FreeMLRecord( MLRecord &record )
{	
	return true;
}
bool   FullCacheConnector::Close()
{
	if (database)
		return database->Close();	
	return true;
}

bool   FullCacheConnector::GetRecordLabel( double &label,UInt32 index )
{
	if (index >= nrRecords) 
	{
		notifier->Error("[%s] -> index is out of range, it has to be less than %d", nrRecords);
		return false;
	}


	label = Labels[index];
	return true;
}
bool   FullCacheConnector::Save(char *fileName)
{
	GML::ML::CacheHeader	h;

	while (true)
	{
		if (CreateCacheFile(fileName,CACHE_SIG_NAME,&h,sizeof(h))==false)
			break;		
		if (file.Write(&h,sizeof(h))==false)
			break;
		if (file.Write(Records,nrRecords*columns.nrFeatures*sizeof(double))==false)
			break;
		if (file.Write(Labels,nrRecords*sizeof(double))==false)
			break;
		if (file.Write(Weights,nrRecords*sizeof(double))==false)
			break;
		if (SaveRecordHashesAndFeatureNames()==false)
			break;
		CloseCacheFile();
		return true;
	}
	notifier->Error("[%s] Unable to write into %s",ObjectName,fileName);
	CloseCacheFile();
	DeleteFileA(fileName);
	return false;
}
bool   FullCacheConnector::Load(char *fileName)
{
	GML::ML::CacheHeader	h;
	while (true)
	{
		if (OpeanCacheFile(fileName,CACHE_SIG_NAME,&h,sizeof(h))==false)
			break;

		if (Records!=NULL)
			delete Records;
		if (Labels!=NULL)
			delete Labels;
		if (Weights!=NULL)
			delete Weights;
		Records = NULL;
		Labels = NULL;
		Weights = NULL;

		if ((Records = new double[nrRecords*columns.nrFeatures])==NULL)
		{
			notifier->Error("[%s] -> Unable to allocate %ud bytes for data indexes !",ObjectName,nrRecords*columns.nrFeatures*sizeof(double));
			break;
		}

		// alloc memory for labels
		if ((Labels = new double[nrRecords])==NULL)
		{
			notifier->Error("[%s] -> Unable to allocate %ud bytes for labels !",ObjectName,nrRecords*sizeof(double));
			break;
		}

		// alloc memory for weights
		if ((Weights = new double[nrRecords])==NULL)
		{
			notifier->Error("[%s] -> Unable to allocate %ud bytes for weights !",ObjectName,nrRecords*sizeof(double));
			break;
		}

		// read records from file
		UInt64 ReadSz;
		if (file.Read(Records, nrRecords*columns.nrFeatures*sizeof(double), &ReadSz)==false) {
			notifier->Error("[%s] -> Unable to read records data!",ObjectName);
			break;
		}
		if (ReadSz!=nrRecords*columns.nrFeatures*sizeof(double)) {
			notifier->Error("[%s] -> Unable to read enough from file!",ObjectName);
			break;
		}

		// read labels from file
		if (file.Read(Labels, nrRecords*sizeof(double), &ReadSz)==false) {
			notifier->Error("[%s] -> Unable to read labels data!",ObjectName);
			break;
		}
		if (ReadSz!=nrRecords*sizeof(double)) {
			notifier->Error("[%s] -> Unable to read enough from file!",ObjectName);
			break;
		}

		// read weights from file
		if (file.Read(Weights, nrRecords*sizeof(double), &ReadSz)==false) {
			notifier->Error("[%s] -> Unable to read labels data!",ObjectName);
			break;
		}
		if (ReadSz!=nrRecords*sizeof(double)) {
			notifier->Error("[%s] -> Unable to read enough from file!",ObjectName);
			break;
		}
		
		if (LoadRecordHashesAndFeatureNames(&h)==false)
			break;
		CloseCacheFile();
		dataMemorySize = nrRecords*sizeof(double);
		return true;		
	}
	ClearColumnIndexes();
	CloseCacheFile();
	if (Records!=NULL)
		delete Records;
	if (Labels!=NULL)
		delete Labels;
	if (Weights!=NULL)
		delete Weights;
	Records = NULL;
	Labels = NULL;
	Weights = NULL;

	return false;
}

bool FullCacheConnector::GetRecordWeight( UInt32 index,double &weight )
{
	if (index >= nrRecords) 
	{
		notifier->Error("[%s] -> index is out of range, it has to be less than %d", nrRecords);
		return false;
	}


	weight = Weights[index];
	return true;	
}
