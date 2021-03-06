#include "gmllib.h"

template <class DataType> class DataTypeTemplateConnector: public GML::ML::IConnector
{
public:
	DataType			*Data;	
	DataType			*Labels;
	GML::Utils::BitSet	BSLabel;
	bool				LabelIsBool;
	char				*SigName;	


	bool				OnInitConnectionToDataBase();
	bool				OnInitConnectionToConnector();
	bool				AllocMemory();
public:
	DataTypeTemplateConnector();

	
	bool		Close();
	bool		Save(char *fileName);
	bool		Load(char *fileName);
	bool		CreateMlRecord (GML::ML::MLRecord &record);
	bool		GetRecord(GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask=0);
	bool		FreeMLRecord(GML::ML::MLRecord &record);
	bool		GetRecordLabel(double &label,UInt32 index);


};
//===================================================================================================
template <class DataType> DataTypeTemplateConnector<DataType>::DataTypeTemplateConnector()
{
	Data = NULL;
	Labels = NULL;
	
	SigName = "DataTypeConnectorV1";
	ObjectName = "DataTypeTemplateConnector";

	LinkPropertyToBool("LabelIsBool",LabelIsBool,false,"Specify if label is consider a bool value or not");

	AddTwoClassLabelProperties();
	AddCacheProperties();
	AddStoreProperties();
}

template <class DataType> bool	DataTypeTemplateConnector<DataType>::AllocMemory()
{
	if ((Data = new DataType[(UInt64)nrRecords*(UInt64)columns.nrFeatures])==NULL)
	{
		notifier->Error("[%s] -> Unable to allocate %ud bytes for data !",ObjectName,nrRecords*columns.nrFeatures);
		return false;
	}
	memset(Data,0,(UInt64)nrRecords*(UInt64)columns.nrFeatures*sizeof(DataType));
	if (!LabelIsBool)
	{
		if ((Labels = new DataType[nrRecords])==NULL) 
		{
			notifier->Error("[%s] -> Unable to allocate %ud bytes for label !",ObjectName,nrRecords);
			return false;
		}
		memset(Labels,0,nrRecords*sizeof(DataType));
	} else {
		if (BSLabel.Create(nrRecords)==false)
		{
			notifier->Error("[%s] -> Unable to allocate %ud bytes for labels bitset!",ObjectName,nrRecords/8);
			return false;
		}
		BSLabel.SetAll(false);
	}
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
template <class DataType> bool	DataTypeTemplateConnector<DataType>::OnInitConnectionToConnector()
{
	UInt32										tr,gr,recMask;	
	DataType									*cPoz;
	GML::ML::MLRecord							cRec;
	bool										Label;

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
				cPoz[gr] = (DataType)cRec.Features[gr];
		// pun si label-ul
		if (LabelIsBool)
		{
			if (UpdateTwoClassLabelValue(cRec.Label,Label)==false)
				return false;
			BSLabel.Set(tr,Label);
		} else {
			Labels[tr] = (DataType)cRec.Label;
		}
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
		cPoz+=columns.nrFeatures;
	}	
	notifier->EndProcent();

	if (DuplicateRecordWeights(conector)==false)
		return false;
	// all ok , am incarcat datele
	dataMemorySize = (UInt64)nrRecords * sizeof(DataType) * columns.nrFeatures;
	if (LabelIsBool)
		dataMemorySize += nrRecords/8;
	else
		dataMemorySize += nrRecords * sizeof(DataType);
	conector->FreeMLRecord(cRec);
	return true;

}
template <class DataType> bool	DataTypeTemplateConnector<DataType>::OnInitConnectionToDataBase()
{
	UInt32										tr,gr;
	GML::Utils::GTFVector<GML::DB::DBRecord>	VectPtr;
	DataType									*cPoz;	
	GML::DB::RecordHash							cHash;
	double										cValue;
	bool										Label;

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
			cPoz[gr]=(DataType)cValue;
		}
		// pun si label-ul
		if (UpdateDoubleValue(VectPtr,columns.indexLabel,cValue)==false)
			return false;
		if (LabelIsBool)
		{
			if (UpdateTwoClassLabelValue(cValue,Label)==false)
				return false;
			BSLabel.Set(tr,Label);
		} else {
			Labels[tr] = (DataType)cValue;
		}
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
	// all ok , am incarcat datele
	dataMemorySize = (UInt64)nrRecords * sizeof(DataType) * columns.nrFeatures;
	if (LabelIsBool)
		dataMemorySize += nrRecords/8;
	else
		dataMemorySize += nrRecords * sizeof(DataType);
	return true;
}

template <class DataType> bool	DataTypeTemplateConnector<DataType>::Close()
{
	if (Data!=NULL)
		delete Data;
	if (Labels!=NULL)
		delete Labels;
	Data = NULL;
	Labels = NULL;

	return true;
}
template <class DataType> bool	DataTypeTemplateConnector<DataType>::Save(char *fileName)
{
	GML::ML::CacheHeader	h;

	while (true)
	{
		if (CreateCacheFile(fileName,SigName,&h,sizeof(h),(UInt32)LabelIsBool)==false)
			break;
		if (file.Write(&h,sizeof(h))==false)
			break;
		if (LabelIsBool)
		{
			if (file.Write(BSLabel.GetData(),BSLabel.GetAllocated())==false)
				break;
		} else {
			if (file.Write(Labels,(UInt64)nrRecords*(UInt64)sizeof(DataType))==false)
				break;
		}
		if (file.Write(Data,(UInt64)nrRecords*(UInt64)sizeof(DataType)*(UInt64)columns.nrFeatures)==false)
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
template <class DataType> bool	DataTypeTemplateConnector<DataType>::Load(char *fileName)
{
	GML::ML::CacheHeader	h;
	while (true)
	{
		if (OpeanCacheFile(fileName,SigName,&h,sizeof(h))==false)
			break;
		if (Labels!=NULL)
			delete Labels;
		if (Data!=NULL)
			delete Data;
		if (h.Flags!=0)
			LabelIsBool = true;
		else
			LabelIsBool = false;
		if ((Data = new DataType[nrRecords*columns.nrFeatures])==NULL)
		{
			notifier->Error("[%s] -> Unable to allocate %ud bytes for data indexes !",ObjectName,nrRecords*columns.nrFeatures);
			break;
		}
		if (LabelIsBool)
		{
			if (BSLabel.Create(nrRecords)==false)
			{
				notifier->Error("[%s] -> Unable to allocate %ud indexes !",ObjectName,nrRecords);
				break;
			}
			if (file.Read(BSLabel.GetData(),BSLabel.GetAllocated())==false)
				break;
		} else {
			if ((Labels = new DataType[nrRecords])==NULL)
			{
				notifier->Error("[%s] -> Unable to allocate %ud indexes !",ObjectName,nrRecords);
				break;
			}
			if (file.Read(Labels,nrRecords*sizeof(DataType))==false)
				break;
		}
		if (file.Read(Data,(UInt64)nrRecords*(UInt64)columns.nrFeatures*(UInt64)sizeof(DataType))==false)
			break;
		if (LoadRecordHashesAndFeatureNames(&h)==false)
			break;
		CloseCacheFile();
		dataMemorySize = (UInt64)nrRecords * (UInt64)sizeof(DataType) * (UInt64)columns.nrFeatures;
		if (LabelIsBool)
			dataMemorySize += nrRecords/8;
		else
			dataMemorySize += nrRecords * sizeof(DataType);
		return true;		
	}
	ClearColumnIndexes();
	CloseCacheFile();
	notifier->Error("[%s] -> Error read data from %s",ObjectName,fileName);
	return false;
}

template <class DataType> bool	DataTypeTemplateConnector<DataType>::CreateMlRecord (GML::ML::MLRecord &record)
{
	record.FeatCount = columns.nrFeatures;	
	return ((record.Features = new double[columns.nrFeatures])!=NULL);
}
template <class DataType> bool	DataTypeTemplateConnector<DataType>::GetRecord(GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask)
{
	DataType	*cPoz = &Data[index*columns.nrFeatures];
	UInt32		tr;

	if (index>=nrRecords)
		return false;

	for (tr=0;tr<columns.nrFeatures;tr++,cPoz++)
	{
		record.Features[tr] = (double)(*cPoz);
	}
	// pun si label-ul
	if (LabelIsBool)
	{
		if (BSLabel.Get(index))
			record.Label = OutLabelPositive;
		else
			record.Label = OutLabelNegative;
	} else {
		record.Label = (double)Labels[index];
	}
	// pun si hash-ul
	if (recordMask & GML::ML::ConnectorFlags::STORE_HASH)
	{
		if (StoreRecordHash)
			record.Hash.Copy(*Hashes.GetPtrToObject(index));
		else
			record.Hash.Reset();
	}

	return true;
}
template <class DataType> bool	DataTypeTemplateConnector<DataType>::GetRecordLabel(double &Label,UInt32 index)
{
	if (index>=nrRecords)
		return false;
	if (LabelIsBool)
	{
		if (BSLabel.Get(index))
			Label = OutLabelPositive;
		else
			Label = OutLabelNegative;
	} else {
		Label = (double)Labels[index];
	}
	return true;
}
template <class DataType> bool	DataTypeTemplateConnector<DataType>::FreeMLRecord(GML::ML::MLRecord &record)
{
	if (record.Features!=NULL)
	{
		delete record.Features;
		record.Features = NULL;
	}
	return true;
}