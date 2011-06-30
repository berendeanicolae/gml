#include "FullCacheConnector.h"

#define CACHE_SIG_NAME "FullCacheConnectorV2"

FullCacheConnector::FullCacheConnector()
{
	 Records = NULL;
	 Labels  = NULL;	 

	 ObjectName = "FullCacheConnector";

	AddDataBaseProperties();
	AddCacheProperties();
	AddStoreProperties();
}
FullCacheConnector::~FullCacheConnector()
{
	if (Records != NULL) 	
		delete Records;

	if (Labels!= NULL)
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
	GML::Utils::GString							tempStr;
	double										cValue;
	
	if (database->Connect()==false)
	{
		notifier->Error("[%s] -> Could not connect to database",ObjectName);
		return false;
	}
	tempStr.SetFormated("%s LIMIT 1",Query.GetText());
	if (UpdateColumnInformations(tempStr.GetText())==false)
		return false;
	if (QueryRecordsCount(CountQuery.GetText(),nrRecords)==false)
		return false;
	if (nrRecords==0) 
	{
		notifier->Error("[%s] -> I received 0 records from the database",ObjectName);
		return false;
	}
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
	memset(Records,0,nrRecords*columns.nrFeatures*sizeof(double));
	memset(Labels,0,nrRecords*sizeof(double));
	// sunt exact la inceput
	cPoz = Records;

	for (tr=0;tr<nrRecords;tr++)
	{
		// cache
		if ((tr % CachedRecords)==0)
		{
			if (tr+CachedRecords<nrRecords)
				tempStr.SetFormated("%s LIMIT %d,%d",Query.GetText(),tr,CachedRecords);
			else
				tempStr.SetFormated("%s LIMIT %d,%d",Query.GetText(),tr,nrRecords-tr);
			//notifier->Info("%s",tempStr.GetText());
			if (database->ExecuteQuery(tempStr.GetText())==false)
			{
				notifier->Error("[%s] -> Unable to Execute query : %s !",ObjectName,tempStr.GetText());
				return false;
			}
		}
		if (database->FetchNextRow(VectPtr)==false)
		{
			notifier->Error("[%s] -> Error reading #%d record !",ObjectName,tr);
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
		// trecem la urmatorul record
		cPoz+=columns.nrFeatures;
	}	
	// all ok , am incarcat datele

	return true;
}
bool   FullCacheConnector::FreeMLRecord( MLRecord &record )
{	
	return true;
}
bool   FullCacheConnector::Close()
{
	if (database)
		return database->Disconnect();	
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
		Records = NULL;
		Labels = NULL;
		if ((Records = new double[nrRecords*columns.nrFeatures*sizeof(double)])==NULL)
		{
			notifier->Error("[%s] -> Unable to allocate %ud bytes for data indexes !",ObjectName,nrRecords*columns.nrFeatures*sizeof(double));
			break;
		}
		if ((Labels = new double[nrRecords*sizeof(double)])==NULL)
		{
			notifier->Error("[%s] -> Unable to allocate %ud bytes for labels !",ObjectName,nrRecords*sizeof(double));
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
	Records = NULL;
	Labels = NULL;

	return false;
}
