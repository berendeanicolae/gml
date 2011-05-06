#include "FullCacheConnector.h"

FullCacheConnector::FullCacheConnector()
{
	 nrRecords = 0;
	 FeatureCount = 0;

	 Records = NULL;
	 Labels  = NULL;	 

	 ObjectName = "FullCacheConnector";

	AddDataBaseProperties();
	AddCacheProperties();
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

UInt32 FullCacheConnector::GetRecordCount()
{
	return nrRecords;
}
UInt32 FullCacheConnector::GetFeatureCount()
{
	return FeatureCount;		
}
bool   FullCacheConnector::GetRecord( MLRecord &record,UInt32 index,UInt32 recordMask )
{
	if (index >= nrRecords) 
	{
		notifier->Error("[%s] -> index is out of range, it has to be less than %d", nrRecords);
		return false;
	}
	record.Features = (double*) &Records[index*FeatureCount]; 
	record.FeatCount = FeatureCount;
	record.Label = Labels[index];	
	
	return true;
}
bool   FullCacheConnector::CreateMlRecord( MLRecord &record )
{	
	record.FeatCount = FeatureCount;
	return true;
}
bool   FullCacheConnector::SetRecordInterval( UInt32 start, UInt32 end )
{
	return false;
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
	FeatureCount = columns.nrFeatures;
	notifier->Info("[%s] -> Records=%d,Features=%d,MemSize=%d",ObjectName,nrRecords,columns.nrFeatures,(nrRecords+1)*sizeof(double)*columns.nrFeatures);
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
UInt32 FullCacheConnector::GetTotalRecordCount()
{
	return nrRecords;
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
	GML::Utils::File	f;

	if (f.Create(fileName)==false)
	{
		notifier->Error("[%s] Unable to create : %s",ObjectName,fileName);
		return false;
	}
	while (true)
	{
		if (f.Write("FullCacheConnectorCache",23)==false)
			break;
		if (f.Write(&nrRecords,sizeof(UInt32))==false)
			break;
		if (f.Write(&columns.nrFeatures,sizeof(UInt32))==false)
			break;
		if (f.Write(Records,nrRecords*columns.nrFeatures*sizeof(double))==false)
			break;
		if (f.Write(Labels,nrRecords*sizeof(double))==false)
			break;
		f.Close();
		return true;
	}
	notifier->Error("[%s] Unable to write into %s",ObjectName,fileName);
	f.Close();
	DeleteFileA(fileName);
	return false;
}
bool   FullCacheConnector::Load(char *fileName)
{
	GML::Utils::File	f;
	char				temp[24];

	notifier->Info("[%s] -> Loading %s",ObjectName,fileName);
	if (f.OpenRead(fileName)==false)
	{
		notifier->Error("[%s] -> Unable to open : %s",ObjectName,fileName);
		return false;
	}
	while (true)
	{
		if (f.Read(temp,23)==false)
			break;
		if (memcmp(temp,"FullCacheConnectorCache",23)!=0)
		{
			notifier->Error("[%s] -> Invalid file format : %s",ObjectName,fileName);
			break;
		}
		if (f.Read(&nrRecords,sizeof(UInt32))==false)
			break;
		if (f.Read(&columns.nrFeatures,sizeof(UInt32))==false)
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
		if (f.Read(Records,nrRecords*columns.nrFeatures*sizeof(double))==false)
			break;
		if (f.Read(Labels,nrRecords*sizeof(double))==false)
			break;
		f.Close();
		FeatureCount = columns.nrFeatures;
		notifier->Info("[%s] -> Records=%d,Features=%d,MemSize=%d",ObjectName,nrRecords,columns.nrFeatures,(nrRecords+1)*columns.nrFeatures*sizeof(double));
		return true;
	}
	if (Records!=NULL)
		delete Records;
	if (Labels!=NULL)
		delete Labels;
	Records = NULL;
	Labels = NULL;
	nrRecords = 0;
	columns.nrFeatures = FeatureCount = 0;

	notifier->Error("[%s] -> Error reading data from %s",ObjectName,fileName);
	f.Close();	
	return false;
}
