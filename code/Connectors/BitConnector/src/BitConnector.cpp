#include "BitConnector.h"

#define CACHE_SIG_NAME "BitConnectorV3"

BitConnector::BitConnector()
{
	Data = NULL;
	ObjectName = "BitConnector";

	AddDataBaseProperties();
	AddCacheProperties();
	AddStoreProperties();
}

bool	BitConnector::AllocMemory()
{
	// aloca data :D
	if (((columns.nrFeatures+1)%8)==0)
		Align8Size = (columns.nrFeatures+1)/8;
	else
		Align8Size = ((columns.nrFeatures+1)/8)+1;

	if ((Data = new UInt8[nrRecords*Align8Size])==NULL)
	{
		notifier->Error("[%s] -> Unable to allocate %ud bytes for data !",ObjectName,nrRecords*Align8Size);
		return false;
	}
	memset(Data,0,nrRecords*Align8Size);
	if (StoreRecordHash)
	{
		if (Hashes.Create(nrRecords)==false)
		{
			notifier->Error("[%s] -> Unable to allocate %ud bytes for hashes !",ObjectName,nrRecords*sizeof(GML::DB::RecordHash));
			return false;
		}
	}
	return true;
}
bool	BitConnector::OnInitConnectionToConnector()
{
	UInt32										tr,gr,recMask;	
	UInt8										*cPoz;
	GML::ML::MLRecord							cRec;

	columns.nrFeatures = conector->GetFeatureCount();
	nrRecords = conector->GetRecordCount();
	if (UpdateFeaturesNameFromConnector()==false)
		return false;
	if (nrRecords==0) 
	{
		notifier->Error("[%s] -> I received 0 records from the parent connector",ObjectName);
		return false;
	}
	if (conector->CreateMlRecord(cRec)==false)
	{
		notifier->Error("[%s] -> Unable to crea MLRecord ",ObjectName);
		return false;
	}
	if (AllocMemory()==false)
		return false;

	// sunt exact la inceput
	cPoz = Data;
	recMask = 0;
	if (StoreRecordHash)
		recMask |= GML::ML::ConnectorFlags::STORE_HASH;

	notifier->StartProcent("[%s] -> Loading Data : ",ObjectName);

	for (tr=0;tr<nrRecords;tr++)
	{
		if ((tr % 1000)==0)
			notifier->SetProcent((double)tr,(double)nrRecords);
		
		if (conector->GetRecord(cRec,tr,recMask)==false)		
		{
			notifier->Error("[%s] -> Error reading #%d record from parent connector!",ObjectName,tr);
			return false;
		}
		// pentru fiecare record pun valorile
		for (gr=0;gr<columns.nrFeatures;gr++)
			if (cRec.Features[gr]==1.0)
				cPoz[gr/8] |= (1<<(gr%8));
		// pun si label-ul
		if (cRec.Label==1.0)
			cPoz[columns.nrFeatures/8] |= (1<<(columns.nrFeatures%8));
		// adaug si Hash-ul
		if (StoreRecordHash)
		{
			//GML::Utils::GString	tmp;
			//cRec.Hash.ToString(tmp);
			//notifier->Info(" %s ",tmp.GetText());
			if (Hashes.PushByRef(cRec.Hash)==false)
			{
				notifier->Error("[%s] -> Unable to save Hash with id %d",ObjectName,tr);
				return false;
			}
		}
		// trecem la urmatorul record
		cPoz+=Align8Size;
	}	
	notifier->EndProcent();
	// all ok , am incarcat datele
	dataMemorySize = (UInt64)nrRecords * (UInt64)Align8Size;
	conector->FreeMLRecord(cRec);
	return true;

}
bool	BitConnector::OnInitConnectionToDataBase()
{
	UInt32										tr,gr;
	GML::Utils::GTFVector<GML::DB::DBRecord>	VectPtr;
	UInt8										*cPoz;
	GML::Utils::GString							tempStr;
	GML::DB::RecordHash							cHash;
	double										cValue;

	if (AllocMemory()==false)
		return false;
	// sunt exact la inceput
	cPoz = Data;

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
			if (cValue==1.0)
				cPoz[gr/8] |= (1<<(gr%8));
		}
		// pun si label-ul
		if (UpdateDoubleValue(VectPtr,columns.indexLabel,cValue)==false)
			return false;
		if (cValue==1.0)
			cPoz[columns.nrFeatures/8] |= (1<<(columns.nrFeatures%8));
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
		cPoz+=Align8Size;
		if ((tr % 1000)==0)
			notifier->SetProcent(tr,nrRecords);
	}	
	notifier->EndProcent();
	// all ok , am incarcat datele
	dataMemorySize = (UInt64)nrRecords * (UInt64)Align8Size;
	return true;
}

bool	BitConnector::Close()
{
	if (Data!=NULL)
		delete Data;
	Data = NULL;
	return true;
}
bool	BitConnector::Save(char *fileName)
{
	BitConnectorHeader	h;

	while (true)
	{
		if (CreateCacheFile(fileName,CACHE_SIG_NAME,&h,sizeof(h))==false)
			break;
		h.Align8Size = Align8Size;
		if (file.Write(&h,sizeof(h))==false)
			break;
		if (file.Write(Data,nrRecords*Align8Size)==false)
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
bool	BitConnector::Load(char *fileName)
{
	BitConnectorHeader	h;
	while (true)
	{
		if (OpeanCacheFile(fileName,CACHE_SIG_NAME,&h,sizeof(h))==false)
			break;
		if (h.Align8Size==0)
			break;
		Align8Size = h.Align8Size;
		if (Data!=NULL)
			delete Data;
		if ((Data = new UInt8[nrRecords*Align8Size])==NULL)
		{
			notifier->Error("[%s] -> Unable to allocate %ud bytes for data indexes !",ObjectName,nrRecords*Align8Size);
			break;
		}
		if (file.Read(Data,nrRecords*Align8Size)==false)
			break;
		if (LoadRecordHashesAndFeatureNames(&h)==false)
			break;
		CloseCacheFile();
		dataMemorySize = nrRecords*Align8Size;
		return true;		
	}
	ClearColumnIndexes();
	CloseCacheFile();
	notifier->Error("[%s] -> Error read data from %s",ObjectName,fileName);
	return false;
}

bool	BitConnector::CreateMlRecord (GML::ML::MLRecord &record)
{
	record.FeatCount = columns.nrFeatures;	
	return ((record.Features = new double[columns.nrFeatures])!=NULL);
}
bool	BitConnector::GetRecord(GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask)
{
	UInt8	*cPoz = &Data[index*Align8Size];
	UInt8	*ptr;
	UInt32	tr;
	UInt8	mask;
	double	*feat = &record.Features[0];

	if (index>=nrRecords)
		return false;
	
	for (tr=0,ptr=cPoz,mask=1;tr<columns.nrFeatures;tr++,feat++)
	{
		if (((*ptr) & mask)!=0)
			(*feat)=1.0;
		else
			(*feat)=0.0;
		mask <<= 1;
		if (mask==0)
		{
			mask = 1;
			ptr++;
		}
	}
	// pun si label-ul
	if ((cPoz[columns.nrFeatures/8] & (1<<(columns.nrFeatures%8)))!=0)
		record.Label = 1.0;
	else
		record.Label = -1.0;

	if (recordMask & GML::ML::ConnectorFlags::STORE_HASH)
	{
		if (StoreRecordHash)
			record.Hash.Copy(*Hashes.GetPtrToObject(index));
		else
			record.Hash.Reset();
	}

	return true;
}
bool	BitConnector::GetRecordLabel(double &Label,UInt32 index)
{
	UInt8	*cPoz = &Data[index*Align8Size];

	if (index>=nrRecords)
		return false;

	// pun si label-ul
	if ((cPoz[columns.nrFeatures/8] & (1<<(columns.nrFeatures%8)))!=0)
		Label = 1.0;
	else
		Label = -1.0;

	return true;
}
bool	BitConnector::FreeMLRecord(GML::ML::MLRecord &record)
{
	if (record.Features!=NULL)
	{
		delete record.Features;
		record.Features = NULL;
	}
	return true;
}
