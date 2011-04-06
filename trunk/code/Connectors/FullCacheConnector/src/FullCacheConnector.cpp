#include "FullCacheConnector.h"

UInt32 FullCacheConnector::GetRecordCount()
{
	return nrRecords;
}

UInt32 FullCacheConnector::GetFeatureCount()
{
	return FeatureCount;		
}

bool FullCacheConnector::GetRecord( MLRecord &record,UInt32 index )
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

bool FullCacheConnector::CreateMlRecord( MLRecord &record )
{	
	record.FeatCount = FeatureCount;
	return true;
}

bool FullCacheConnector::SetRecordInterval( UInt32 start, UInt32 end )
{
	return false;
}

bool FullCacheConnector::OnInit()
{
	UInt32										tr,gr;
	GML::Utils::GTFVector<GML::DB::DBRecord>	VectPtr;
	double										*cPoz;
	GML::Utils::GString							tempStr;
	double										cValue;

	// daca iau datele din cache
	if ((database==NULL) && (conector==NULL))
		return true;
	if (database==NULL)
	{
		notifier->Error("[%s] works with a filedata or a database",ObjectName);
		return false;
	}
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
			if (cValue==1.0)
				cPoz[gr] = 1.0;
		}
		// pun si label-ul
		if (UpdateDoubleValue(VectPtr,columns.indexLabel,cValue)==false)
			return false;
		if (cValue==1.0)
			Labels[tr] = 1.0;
		else
			Labels[tr] = -1.0;
		// trecem la urmatorul record
		cPoz+=columns.nrFeatures;
	}	
	// all ok , am incarcat datele
	FeatureCount = columns.nrFeatures;
	notifier->Info("[%s] -> Records=%d,Features=%d,MemSize=%d",ObjectName,nrRecords,columns.nrFeatures,(nrRecords+1)*sizeof(double)*columns.nrFeatures);
	return true;
}

bool FullCacheConnector::FreeMLRecord( MLRecord &record )
{	
	return true;
}

bool FullCacheConnector::Close()
{
	if (database)
		return database->Disconnect();	
	return true;
}

UInt32 FullCacheConnector::GetTotalRecordCount()
{
	return nrRecords;
}

FullCacheConnector::FullCacheConnector()
{
	 nrRecords = 0;
	 FeatureCount = 0;

	 Records = NULL;
	 Labels  = NULL;	 

	 ObjectName = "FullCacheConnector";
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

bool FullCacheConnector::GetRecordLabel( double &label,UInt32 index )
{
	if (index >= nrRecords) 
	{
		notifier->Error("[%s] -> index is out of range, it has to be less than %d", nrRecords);
		return false;
	}


	label = Labels[index];
	return true;
}
