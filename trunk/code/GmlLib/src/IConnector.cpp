#include "IConnector.h"

/*
				RECORD_WEIGHT_UINT8 = 0,
				RECORD_WEIGHT_UINT16 = 1,
				RECORD_WEIGHT_UINT32 = 2,
				RECORD_WEIGHT_UINT64 = 3,
				RECORD_WEIGHT_DOUBLE = 4,
*/
static UInt32 weightDataTypeSize[]= {1,2,4,8,8};

void ConnectorThreadRedirectFunction(GML::Utils::IParalelUnit *paralel,void *context)
{
	GML::ML::IConnector *con = (GML::ML::IConnector *)context;
	con->OnRunThreadCommand(con->ThData[paralel->GetID()],paralel->GetCodeID());
}

GML::ML::IConnector::IConnector()
{
	notifier = NULL;
	database = NULL;
	conector = NULL;
	connectors = NULL;
	connectorsCount = 0;
	ObjectName = "IConnector";
	columns.indexFeature = NULL;
	ClearColumnIndexes();
	nrRecords = 0;
	dataMemorySize = 0;
	RecordsWeight = NULL;	
	StoreRecordWeightMode = 0;
	tpu = NULL;
	threadsCount = 1;
}
void GML::ML::IConnector::AddMultiThreadingProperties()
{
	LinkPropertyToUInt32("ThreadsCount"				,threadsCount			,1,"Number of threads to be used for parallel computations.");
}
void GML::ML::IConnector::AddTwoClassLabelProperties()
{
	LinkPropertyToDouble("InLabelPositive",InLabelPositive,1.0,"Value for the positive Label received from the database or a conector");
	LinkPropertyToDouble("InLabelNegative",InLabelNegative,-1.0,"Value for the positive Label received from the database or a conector");
	LinkPropertyToDouble("OutLabelPositive",OutLabelPositive,1.0,"Value for the label that will be set when querying a positive label");
	LinkPropertyToDouble("OutLabelNegative",OutLabelNegative,-1.0,"Value for the label that will be set when querying a negative label");
	LinkPropertyToUInt32("LabelConversionMethod",LabelConversionMethod,LABEL_CONVERT_EXACT,"!!LIST:Exact=0,ExactPositive,ExactNegative!!");
}
void GML::ML::IConnector::AddCacheProperties()
{
	LinkPropertyToString("DataFileName",DataFileName,"","Name of the file that contains data to be loaded");
}
void GML::ML::IConnector::AddStoreProperties()
{
	LinkPropertyToBool  ("StoreRecordHash",StoreRecordHash,false,"Specify if the connector should store records hash or not");
	LinkPropertyToBool  ("StoreFeatureName",StoreFeaturesName,false,"Specify if the connector should store features name or not");
	LinkPropertyToBool	("StoreRecordWeight",StoreRecordWeight,false,"Specify if the connector should store record weights or not");
}
void GML::ML::IConnector::ClearColumnIndexes()
{
	if (columns.indexFeature!=NULL)
		delete columns.indexFeature;
	columns.indexFeature = NULL;
	columns.nrFeatures = 0;
	columns.indexLabel = -1;
	columns.indexHash = -1;
	dataFeaturesNames.DeleteAll();
	indexFeatureNames.DeleteAll();
}
bool GML::ML::IConnector::AddColumnName(char *name)
{
	UInt32	poz = dataFeaturesNames.Len();
	UInt8	ch;

	if (name==NULL)
	{
		notifier->Error("[%s] -> NULL column name!",ObjectName);
		return false;
	}
	if (indexFeatureNames.Push(poz)==false)
	{
		notifier->Error("[%s] -> Unable to allocate memory for column with name: %s !",ObjectName,name);
		return false;
	}
	for (int tr=0;name[tr]!=0;tr++)
	{
		ch = name[tr];
		if (dataFeaturesNames.Push(ch)==false)
		{
			notifier->Error("[%s] -> Unable to allocate memory for column with name: %s !",ObjectName,name);
			return false;
		}
	}
	ch = 0;
	if (dataFeaturesNames.Push(ch)==false)
	{
		notifier->Error("[%s] -> Unable to allocate memory for column with name: %s !",ObjectName,name);
		return false;
	}
	return true;
}
bool GML::ML::IConnector::UpdateFeaturesNameFromConnector()
{
	UInt32					tr;
	GML::Utils::GString		featName;

	indexFeatureNames.DeleteAll();
	dataFeaturesNames.DeleteAll();
	if (StoreFeaturesName==false)
		return true;
	if (conector==NULL)
	{
		notifier->Error("[%s] -> NULL conector , exiting ...",ObjectName);
		return false;
	}
	if (columns.nrFeatures != conector->GetFeatureCount())
	{
		notifier->Error("[%s] -> Invalid number of features (%d) , expecting (%d)",ObjectName,columns.nrFeatures,conector->GetFeatureCount());
		return false;
	}
	
	for (tr=0;tr<columns.nrFeatures;tr++)
	{
		if (conector->GetFeatureName(featName,tr)==false)
		{
			notifier->Error("[%s] -> Unable to read feature #%d",ObjectName,tr);
			return false;
		}
		if (AddColumnName(featName.GetText())==false)			
		{
			notifier->Error("[%s] -> Unable to add column (%s)",ObjectName,featName.GetText());
			return false;
		}
	}
	return true;
}
bool GML::ML::IConnector::UpdateColumnInformationsFromDataBase()
{
	GML::Utils::GTFVector<GML::DB::ColumnInfo>*	db_columns;
	GML::DB::ColumnInfo*						ci;
	UInt32										cPoz;

	if (database == NULL)
	{
		notifier->Error("[%s] -> 'UpdateColumnInformationsFromDataBase' should be used only when connected to a database !",ObjectName);
		return false;
	}
	nrRecords = database->GetRecordCount();
	if (nrRecords == 0)
	{
		notifier->Error("[%s] -> NULL number of records in database",ObjectName);
		return false;
	}
	db_columns = database->GetColumns();
	if (db_columns==NULL)
	{
		notifier->Error("[%s] -> Internal error: NULL pointer for colums in database",ObjectName);
		return false;
	}
	ClearColumnIndexes();
	// analizez datele
	for (UInt32 tr=0;tr<db_columns->Len();tr++)
	{
		ci = db_columns->GetPtrToObject(tr);
		// pentru label
		if (ci->ColumnType == GML::DB::COLUMNTYPE::LABEL)
		{
			if ((ci->DataType!=GML::DB::TYPES::DOUBLE) && 
				(ci->DataType!=GML::DB::TYPES::BOOLEAN) && 
				(ci->DataType!=GML::DB::TYPES::INT16) && 
				(ci->DataType!=GML::DB::TYPES::INT32))
			{
				notifier->Error("[%s] -> Invalid type for Label at column #%d. Allowed types: BOOL,INT16,INT32,DOUBLE !",ObjectName,tr);
				return false;
			}
			columns.indexLabel = tr;
			continue;
		}
		// pentru Hash
		if (ci->ColumnType == GML::DB::COLUMNTYPE::HASH)
		{
			if ((ci->DataType!=GML::DB::TYPES::HASH) && 
				(ci->DataType!=GML::DB::TYPES::ASCII))
			{
				notifier->Error("[%s] -> Invalid type for Hash at column #%d. Allowed types: ASCIIVAL,HASHVAL !",ObjectName,tr);
				return false;
			}
			columns.indexHash = tr;
			continue;
		}
		// pentru Features
		if (ci->ColumnType == GML::DB::COLUMNTYPE::FEATURE)
		{
			if ((ci->DataType!=GML::DB::TYPES::DOUBLE) && 
				(ci->DataType!=GML::DB::TYPES::BOOLEAN) && 
				(ci->DataType!=GML::DB::TYPES::INT8) && 
				(ci->DataType!=GML::DB::TYPES::INT16) && 
				(ci->DataType!=GML::DB::TYPES::INT32) &&
				(ci->DataType!=GML::DB::TYPES::UINT8) && 
				(ci->DataType!=GML::DB::TYPES::UINT16) && 
				(ci->DataType!=GML::DB::TYPES::UINT32) 
				)
			{
				notifier->Error("[%s] -> Invalid type for Feature at column #%d. Allowed types: BOOL,INT8,INT16,INT32,UINT8,UINT16,UINT32,DOUBLE !",ObjectName,tr);
				return false;
			}
			columns.nrFeatures++;
			continue;
		}
	}
	if (columns.nrFeatures==0)
	{
		notifier->Error("[%s] -> Missing Feature from the column header !",ObjectName);
		return false;
	}
	if (columns.indexLabel==-1)
	{
		notifier->Error("[%s] -> Missing Label from the column header !",ObjectName);
		return false;
	}
	// aloc indexii
	if ((columns.indexFeature = new Int32[columns.nrFeatures])==NULL)
	{
		notifier->Error("[%s] -> Unable to alloc %d features indexes ",ObjectName,columns.nrFeatures);
		return false;
	}
	// aloc si indexii
	cPoz = 0;
	for (UInt32 tr=0;tr<db_columns->Len();tr++)
	{
		ci = db_columns->GetPtrToObject(tr);
		if (ci->ColumnType == GML::DB::COLUMNTYPE::FEATURE)
		{
			columns.indexFeature[cPoz] = tr;
			if (StoreFeaturesName)
			{
				if (AddColumnName(ci->Name)==false)			
				{
					notifier->Error("[%s] -> Unable to save name for feature #%d (%s)",ObjectName,tr,ci->Name);
					return false;
				}
			}
			cPoz++;	
		}
	}

	return true;
}
bool GML::ML::IConnector::UpdateTwoClassLabelValue(double value,bool &label)
{
	switch (LabelConversionMethod)
	{
		case LABEL_CONVERT_EXACT:
			if (value==InLabelPositive)
			{
				label = true;
				return true;
			}
			if (value==InLabelNegative)
			{
				label = false;
				return true;
			}
			notifier->Error("[%s] -> Invalid value for label (%lf). Expecting (%lf for positive and %lf for negative).",ObjectName,value,InLabelPositive,InLabelNegative);
			return false;
		case LABEL_CONVERT_FIX_POSITIVE:
			if (value==InLabelPositive)
				label = true;
			else
				label = false;
			return true;	
		case LABEL_CONVERT_FIX_NEGATIVE:
			if (value==InLabelNegative)
				label = false;
			else
				label = true;
			return true;
	}
	notifier->Error("[%s] -> Invalid 'LabelConversionMethod' property value (%d)",ObjectName,LabelConversionMethod);
	return false;
}
bool GML::ML::IConnector::UpdateDoubleValue(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr,Int32 index,double &value)
{
	GML::DB::DBRecord	*rec;

	if ((rec = VectPtr.GetPtrToObject(index))==NULL)
	{
		notifier->Error("[%s] -> Unable to read record with index #%d",ObjectName,index);
		return false;
	}
	switch (rec->Type)
	{
		case GML::DB::TYPES::DOUBLE:
			value = rec->Value.DoubleVal;
			break;
		case GML::DB::TYPES::INT8:
			value = (double)rec->Value.Int8Val;
			break;
		case GML::DB::TYPES::INT16:
			value = (double)rec->Value.Int16Val;
			break;
		case GML::DB::TYPES::INT32:
			value = (double)rec->Value.Int32Val;
			break;
		case GML::DB::TYPES::UINT8:
			value = (double)rec->Value.UInt8Val;
			break;
		case GML::DB::TYPES::UINT16:
			value = (double)rec->Value.UInt16Val;
			break;
		case GML::DB::TYPES::UINT32:
			value = (double)rec->Value.UInt32Val;
			break;
		case GML::DB::TYPES::BOOLEAN:
			if (rec->Value.BoolVal==false)
				value = 0.0;
			else
				value = 1.0;
			break;
		default:
			notifier->Error("[%s] -> Unable to convert column from index %d to double !",ObjectName,index);
			return false;
	}
	return true;
}
bool GML::ML::IConnector::UpdateHashValue(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr,Int32 index,GML::DB::RecordHash &recHash)
{
	GML::DB::DBRecord	*rec;

	if ((rec = VectPtr.GetPtrToObject(index))==NULL)
	{
		notifier->Error("[%s] -> Unable to read record with index #%d",ObjectName,index);
		return false;
	}
	switch (rec->Type)
	{
		case GML::DB::TYPES::ASCII:
			if (recHash.CreateFromText(rec->Value.AsciiStrVal)==false)
			{
				notifier->Error("[%s] -> Unable to convert '%s' to a valid hash !",ObjectName,rec->Value.AsciiStrVal);
				return false;
			}
			break;
		case GML::DB::TYPES::HASH:
			recHash.Copy(rec->Value.Hash);
			break;
		default:
			notifier->Error("[%s] -> Unable to convert column from index %d to double !",ObjectName,index);
			return false;
	}
	return true;
}
bool GML::ML::IConnector::InitThreads()
{
	UInt32	tr;

	if (notifier==NULL)
	{
		DEBUGMSG("[%s] -> Connection to a notifier is not set ...",ObjectName);
		return false;
	}
	if (threadsCount<1)
	{
		notifier->Error("[%s] -> Invalid number of threads (%d). Should be at least one thread.",threadsCount);
		return false;
	}
	if ((ThData = new GML::ML::ConnectorThreadData[threadsCount])==NULL)
	{
		notifier->Error("[%s] -> Unable to create %d MLThreadData Objects",threadsCount,threadsCount);
		return false;
	}
	if ((tpu = new GML::Utils::ThreadParalelUnit[threadsCount])==NULL)
	{
		notifier->Error("[%s] -> Unable to create %d threads ",ObjectName,threadsCount);
		return false;
	}
	for (tr=0;tr<threadsCount;tr++)
	{
		if (tpu[tr].Init(tr,this,ConnectorThreadRedirectFunction)==false)
		{
			notifier->Error("[%s] -> Unable to start thread #%d",ObjectName,tr);
			return false;
		}
		if (conector!=NULL)
		{
			if (conector->CreateMlRecord(ThData[tr].Record)==false)
			{
				notifier->Error("[%s] -> Unable to create MLThreadData[%d].Record",ObjectName,tr);
				return false;
			}
		}
		ThData[tr].ThreadID = tr; 
		ThData[tr].Context = NULL;
		if (OnInitThreadData(ThData[tr])==false)
		{
			notifier->Error("[%s] -> Unable to create MLThreadData.Context ",ObjectName);
			return false;
		}
	}
	notifier->Info("[%s] -> Computation Threads created: %d",ObjectName,threadsCount);
	return true;
}
bool GML::ML::IConnector::Init(GML::Utils::INotifier &_notifier,char *attributeString)
{
	bool										result;

	notifier = &_notifier;

	if ((attributeString!=NULL) && (attributeString[0]!=0))
	{
		if (SetProperty(attributeString)==false)
		{
			notifier->Error("[%s] -> Invalid format for Conector initializations: %s",ObjectName,attributeString);
			return false;
		}
	}

	ClearColumnIndexes();

	notifier->Info("[%s] -> OnInit()",ObjectName);
	result = OnInit();
	if (result==false)
		notifier->Error("[%s] -> OnInit() returned false",ObjectName);
	return result;
}
bool GML::ML::IConnector::Save(char *fileName)
{
	if (notifier)
		notifier->Error("[%s] -> Save function not implemented ",ObjectName);
	return false;
}
bool GML::ML::IConnector::Load(char *fileName)
{
	if (notifier)
		notifier->Error("[%s] Load function not implemented ",ObjectName);
	return false;
}
bool GML::ML::IConnector::OnInitConnectionToDataBase()
{
	if (notifier)
		notifier->Error("[%s] Connection to DataBase is not suported !",ObjectName);
	return false;
}
bool GML::ML::IConnector::OnInitConnectionToConnector()
{
	if (notifier)
		notifier->Error("[%s] -> Connection to another connector is not suported !",ObjectName);
	return false;
}
bool GML::ML::IConnector::OnInitConnectionToCache()
{
	if (notifier==NULL)
	{
		DEBUGMSG("[%s] -> Notifier should be set first before executing this function !",ObjectName);
		return false;
	}
	if (DataFileName.Len()==0)
	{
		notifier->Error("[%s] -> Property 'DataFileName' was not set !",ObjectName);
		return false;
	}
	return Load(DataFileName.GetText());
}
bool GML::ML::IConnector::OnInitThreadData(GML::ML::ConnectorThreadData &thData)
{
	return true;
}
void GML::ML::IConnector::OnRunThreadCommand(GML::ML::ConnectorThreadData &thData,UInt32 threadCommand)
{
}
bool GML::ML::IConnector::ExecuteParalelCommand(UInt32 command)
{
	UInt32	tr;

	if ((tpu==NULL) || (threadsCount<1))
	{
		notifier->Error("[%s] -> Thread data was not initilized (Did you call InitThreads method ?)",ObjectName);
		return false;
	}
	// executie
	for (tr=0;tr<threadsCount;tr++)
		if (tpu[tr].Execute(command)==false)
		{
			notifier->Error("[%s] -> Error on runnig thread #%d",ObjectName,tr);
			return false;
		}
	// asteptare
	for (tr=0;tr<threadsCount;tr++)
		if (tpu[tr].WaitToFinish()==false)
		{
			notifier->Error("[%s] -> WaitToFinish failed on thread #%d",ObjectName,tr);
			return false;
		}
	// all ok
	return true;
}
bool GML::ML::IConnector::OnInit()
{
	GML::Utils::GString		temp;
	UInt64					sz;
	dataMemorySize = 0;
	if (notifier==NULL)
	{
		DEBUGMSG("[%s] -> Notifier should be set first before executing this function !",ObjectName);
		return false;
	}
	ClearColumnIndexes();
	while (true)
	{
		// daca exista o baza de date , incerc conectarea la ea
		if (database!=NULL)
		{
			// initializez baza de date
			if (UpdateColumnInformationsFromDataBase()==false)
				return false;
			if (database->BeginIteration()==false)
			{
				notifier->Error("[%s] -> Failed on BeginIteration()",ObjectName);
				return false;
			}
			if (OnInitConnectionToDataBase()==false)
				return false;
			break;
		}
		// daca exista un connector ma conectez la el
		if (conector!=NULL)
		{
			if (HasProperty("ThreadsCount"))
			{
				if (InitThreads()==false)
					return false;
			}
			if (OnInitConnectionToConnector()==false)
				return false;
			break;
		}
		// altfel incerc si cu cache-ul
		if (OnInitConnectionToCache()==false)
			return false;
		break;
	}
	// afisez si datele
	if (notifier)
	{
		temp.Set("");
		temp.AddFormatedEx("[%{str}] -> Init ok (Records = %{uint32,dec,G3} , Features = %{uint32,dec,G3} , DataSize = %{uint64,dec,G3} bytes)",ObjectName,nrRecords,columns.nrFeatures,dataMemorySize);
		notifier->Info("%s",temp.GetText());

		temp.Set("");
		temp.AddFormatedEx("[%{str}] -> Hashes memory: %{uint32,dec,G3} bytes",ObjectName,Hashes.Len()*sizeof(GML::DB::RecordHash));
		notifier->Info("%s",temp.GetText());

		temp.Set("");
		temp.AddFormatedEx("[%{str}] -> Features name memory: %{uint32,dec,G3} bytes",ObjectName,dataFeaturesNames.Len());
		notifier->Info("%s",temp.GetText());

		temp.Set("");
		sz = 0;
		if ((StoreRecordWeight) && (RecordsWeight!=NULL))
		{
			sz = (UInt64)weightDataTypeSize[StoreRecordWeight]*(UInt64)nrRecords;
		}
		temp.AddFormatedEx("[%{str}] -> Record Weights memory: %{uint64,dec,G3} bytes",ObjectName,sz);
		notifier->Info("%s",temp.GetText());				
	}
	return true;
}
bool GML::ML::IConnector::AllowConnectors(UInt32 count)
{
	return (bool)(count==1);
}
bool GML::ML::IConnector::GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index)
{
	recHash.Reset();

	// daca are conectorul meu
	if ((Hashes.Len()==0) && (conector!=NULL))
		return conector->GetRecordHash(recHash,index);
	if (index>=Hashes.Len())
	{
		notifier->Error("[%s] -> Invalid index (%d) for hash name. Should be within [0..%d]",ObjectName,index,Hashes.Len()-1);
		return true;
	}
	recHash.Copy(Hashes[index]);
	return true;
}
bool GML::ML::IConnector::GetFeatureName(GML::Utils::GString &str,UInt32 index)
{
	char *name = "";
	if (index>=GetFeatureCount())
	{
		notifier->Error("[%s] -> Invalid index (%d) for feature name. Should be within [0..%d]",ObjectName,index,GetFeatureCount()-1);
		return false;
	}
	// daca sunt conectat la un alt 
	if ((indexFeatureNames.Len()==0) && (conector!=NULL))
		return conector->GetFeatureName(str,index);
	
	// altfel dau din lista mea
	if (index<indexFeatureNames.Len())
		name = (char *)dataFeaturesNames.GetPtrToObject(indexFeatureNames[index]);
	if (str.Set(name)==false)
	{
		notifier->Error("[%s] -> Unable to set feature name : %s",ObjectName,name);
		return false;
	}
	return true;
}
UInt32 GML::ML::IConnector::GetRecordCount()
{
	return nrRecords;
}
UInt32 GML::ML::IConnector::GetTotalRecordCount()
{
	return nrRecords;
}
UInt32 GML::ML::IConnector::GetFeatureCount()
{
	return columns.nrFeatures;
}
bool GML::ML::IConnector::AllocRecordsWeight(UInt32 dataWeightType)
{
	if (RecordsWeight!=NULL)
	{
		delete RecordsWeight;
		RecordsWeight = NULL;
	}
	UInt32 size = weightDataTypeSize[dataWeightType];	
	RecordsWeight = new UInt8[nrRecords *size];
	if (RecordsWeight == NULL)
	{
		if (notifier)
			notifier->Error("[%s] -> Unable to alloc %d records for record weight !",ObjectName,nrRecords);
		return false;	
	}
	MEMSET(RecordsWeight,0,nrRecords *size);
	StoreRecordWeightMode = dataWeightType;
	return true;
}
bool GML::ML::IConnector::SetRecordWeight(UInt32 index,double weight)
{
	if (index>=nrRecords) 
	{
		if (notifier)
			notifier->Error("[%s] -> SetRecordWeight - Invalid index: %d",ObjectName,index);
		return false;	
	}
	if (StoreRecordWeight)
	{
		if (notifier)
			notifier->Error("[%s] -> SetRecordWeight::RecordsWeight has not been initilized !",ObjectName);
		return false;		
	}
	switch (StoreRecordWeightMode)
	{
		case GML::ML::ConnectorFlags::RECORD_WEIGHT_UINT8:  
			((UInt8 *)RecordsWeight)[index] = (UInt8)weight; 
			break;
		case GML::ML::ConnectorFlags::RECORD_WEIGHT_UINT16: 
			((UInt16 *)RecordsWeight)[index] = (UInt16)weight; 
			break;
		case GML::ML::ConnectorFlags::RECORD_WEIGHT_UINT32: 
			((UInt32 *)RecordsWeight)[index] = (UInt32)weight; 
			break;
		case GML::ML::ConnectorFlags::RECORD_WEIGHT_UINT64: 
			((UInt64 *)RecordsWeight)[index] = (UInt64)weight; 
			break;
		case GML::ML::ConnectorFlags::RECORD_WEIGHT_DOUBLE: 
			((double *)RecordsWeight)[index] = (double)weight; 
			break;
		default:
			if (notifier)
				notifier->Error("[%s] -> SetRecordWeight::Invalid data type %d !",ObjectName,StoreRecordWeightMode);
			return false;		
	}
	return true;
}
bool GML::ML::IConnector::GetRecordWeight(UInt32 index,double &weight)
{
	if (index>=nrRecords) 
	{
		if (notifier)
			notifier->Error("[%s] -> SetRecordWeight - Invalid index: %d",ObjectName,index);
		return false;	
	}
	if (StoreRecordWeight)
	{
		if (notifier)
			notifier->Error("[%s] -> SetRecordWeight::RecordsWeight has not been initilized !",ObjectName);
		return false;		
	}
	switch (StoreRecordWeightMode)
	{
		case GML::ML::ConnectorFlags::RECORD_WEIGHT_UINT8:  
			weight = ((UInt8 *)RecordsWeight)[index]; 
			break;
		case GML::ML::ConnectorFlags::RECORD_WEIGHT_UINT16: 
			weight = ((UInt16 *)RecordsWeight)[index]; 
			break;
		case GML::ML::ConnectorFlags::RECORD_WEIGHT_UINT32: 
			weight = ((UInt32 *)RecordsWeight)[index]; 
			break;
		case GML::ML::ConnectorFlags::RECORD_WEIGHT_UINT64: 
			weight = ((UInt64 *)RecordsWeight)[index]; 
			break;
		case GML::ML::ConnectorFlags::RECORD_WEIGHT_DOUBLE: 
			weight = ((double *)RecordsWeight)[index]; 
			break;
		default:
			weight = 0;
			if (notifier)
				notifier->Error("[%s] -> SetRecordWeight::Invalid data type %d !",ObjectName,StoreRecordWeightMode);
			return false;		
	}
	return true;
}
bool GML::ML::IConnector::CreateCacheFile(char *fileName,char *sigName,CacheHeader *header,UInt32 headerSize,UInt32 extraFlags)
{
	if ((fileName==NULL) || (header==NULL))
	{
		if (notifier)
			notifier->Error("[%s] -> Null pointer for fileName or header field !",ObjectName);
		return false;
	}
	if (notifier)
		notifier->Info("[%s] -> Saving cache to: %s",ObjectName,fileName);
	if (headerSize<sizeof(CacheHeader))
	{
		if (notifier)
			notifier->Error("[%s] -> Header should be at least %d bytes!",ObjectName,sizeof(CacheHeader));
		return false;
	}
	// creez hederul
	MEMSET(header,0,headerSize);
	if (GML::Utils::GString::Set(header->MagicName,sigName,31)==false)
	{
		if (notifier)
			notifier->Error("[%s] -> SigName should be at least 31 bytes",ObjectName);
		return false;
	}
	header->nrRecords = nrRecords;
	header->nrFeatures = columns.nrFeatures;
	header->Flags = extraFlags;
	if (StoreRecordHash)
		header->StoreFlags |= GML::ML::ConnectorFlags::STORE_HASH;
	if (StoreFeaturesName)
		header->StoreFlags |= GML::ML::ConnectorFlags::STORE_FEATURE_NAME;
	if (StoreRecordWeight)
	{
		header->StoreFlags |= GML::ML::ConnectorFlags::STORE_FEATURE_NAME;
		header->StoreFlags |= ((StoreRecordWeightMode & 7) << 3); 
	}
	

	if (file.Create(fileName)==false)
	{
		if (notifier)
			notifier->Error("[%s] -> Unable to create : %s",ObjectName,fileName);
		return false;
	}
	return true;
}
bool GML::ML::IConnector::OpeanCacheFile(char *fileName,char *sigName,CacheHeader *header,UInt32 headerSize)
{
	if ((fileName==NULL) || (header==NULL))
	{
		if (notifier)
			notifier->Error("[%s] -> Null pointer for fileName or header field !",ObjectName);
		return false;
	}
	if (notifier)
		notifier->Info("[%s] -> Loading cache from: %s",ObjectName,fileName);
	if (headerSize<sizeof(CacheHeader))
	{
		if (notifier)
			notifier->Error("[%s] -> Header should be at least %d bytes!",ObjectName,sizeof(CacheHeader));
		return false;
	}
	if (file.OpenRead(fileName,true)==false)
	{
		if (notifier)
			notifier->Error("[%s] -> Unable to open : %s",ObjectName,fileName);
		return false;
	}
	if (file.Read(header,headerSize)==false)
	{
		if (notifier)
			notifier->Error("[%s] -> Unable to read cache header from %s",ObjectName,fileName);
		return false;
	}
	// creez hederul
	header->MagicName[31]=0;
	if (GML::Utils::GString::Equals(sigName,header->MagicName)==false)
	{
		if (notifier)
			notifier->Error("[%s] -> Invalid header in %s (expecting %s , found %s)",ObjectName,fileName,sigName,header->MagicName);
		return false;
	}
	if (header->nrRecords==0)
	{
		if (notifier)
			notifier->Error("[%s] -> Invalid number of records(%d) in %s",ObjectName,header->nrRecords,fileName);
		return false;
	}
	if (header->nrFeatures==0)
	{
		if (notifier)
			notifier->Error("[%s] -> Invalid number of features(%d) in %s",ObjectName,header->nrFeatures,fileName);
		return false;
	}
	nrRecords = header->nrRecords;
	columns.nrFeatures = header->nrFeatures;
	// resetez unele date	
	return true;
}
void GML::ML::IConnector::CloseCacheFile()
{
	file.Close();
}
bool GML::ML::IConnector::SkipRecordHashes()
{
	UInt64	cPoz;
	if (file.GetFilePos(cPoz)==false)
		return false;
	cPoz += (UInt64)nrRecords*sizeof(GML::DB::RecordHash);
	return file.SetFilePos(cPoz);
}
bool GML::ML::IConnector::SkipRecordWeights()
{
	UInt64	cPoz;
	if (file.GetFilePos(cPoz)==false)
		return false;
	cPoz += (UInt64)nrRecords*weightDataTypeSize[StoreRecordWeightMode];
	return file.SetFilePos(cPoz);
}
bool GML::ML::IConnector::SaveRecordHashes()
{
	// daca nu trebuie sa salvez astea , ies cu true
	if (StoreRecordHash==false)
		return true;
	if (Hashes.Len() != nrRecords)
	{
		if (notifier)
			notifier->Error("[%s] -> Number of elements in the Hashes array is different from total number of records !",ObjectName);
		return false;
	}
	if (file.Write(Hashes.GetPtrToObject(0),nrRecords*sizeof(GML::DB::RecordHash))==false)
	{
		if (notifier)
			notifier->Error("[%s] -> Unable to write to file total number of records",ObjectName);
		return false;
	}
	return true;
}
bool GML::ML::IConnector::LoadRecordHashes()
{
	if (Hashes.Create(nrRecords,true)==false)
	{
		if (notifier)
			notifier->Error("[%s] -> Unable to alloc %d hash entries !",ObjectName,nrRecords);
		return false;
	}
	if (file.Read(Hashes.GetPtrToObject(0),nrRecords*sizeof(GML::DB::RecordHash))==false)
	{
		if (notifier)
			notifier->Error("[%s] -> Unable to read %d record hashes from file",ObjectName,nrRecords);
		return false;
	}
	return true;
}
bool GML::ML::IConnector::SaveRecordWeights()
{
	// daca nu trebuie sa salvez astea , ies cu true
	if (StoreRecordWeight==false)
		return true;
	if (RecordsWeight == NULL)
	{
		if (notifier)
			notifier->Error("[%s] -> RecordsWeight is NULL !",ObjectName);
		return false;
	}
	if (file.Write(RecordsWeight,(UInt64)weightDataTypeSize[StoreRecordWeightMode]*nrRecords)==false)
	{
		if (notifier)
			notifier->Error("[%s] -> Unable to write to file total records weights !",ObjectName);
		return false;
	}
	return true;
}
bool GML::ML::IConnector::LoadRecordWeights()
{
	if (AllocRecordsWeight(StoreRecordWeightMode)==false)
		return false;
	if (file.Read(RecordsWeight,(UInt64)weightDataTypeSize[StoreRecordWeightMode]*nrRecords)==false)
	{
		if (notifier)
			notifier->Error("[%s] -> Unable to read %d record weights from file",ObjectName,nrRecords);
		return false;
	}		
	return true;
}
bool GML::ML::IConnector::DuplicateRecordWeights(GML::ML::IConnector *con)
{
	if (StoreRecordWeight==false)
		return true;
	if (con==NULL)
	{
		if (notifier)
			notifier->Error("[%s] -> Invalid connector pointer",ObjectName);
		return false;	
	}
	if (nrRecords!=con->nrRecords)
	{
		if (notifier)
			notifier->Error("[%s] -> Invalid nrRecords (curent=%d, conector=%d)",ObjectName,nrRecords,con->nrRecords);
		return false;	
	}
	if (AllocRecordsWeight(con->StoreRecordWeightMode)==false)
		return false;
	MEMCOPY(RecordsWeight,con->RecordsWeight,weightDataTypeSize[con->StoreRecordWeightMode]*nrRecords);
	StoreRecordWeightMode = con->StoreRecordWeightMode;
	return true;
}
bool GML::ML::IConnector::SaveFeatureNames()
{
	UInt32	sz;

	// daca nu trebuie sa salvez astea , ies cu true
	if (StoreFeaturesName==false)
		return true;

	if (indexFeatureNames.Len() != columns.nrFeatures)
	{
		if (notifier)
			notifier->Error("[%s] -> Number of elements in the indexFeatureNames array is different from total number of features !",ObjectName);
		return false;
	}
	while (true)
	{
		sz = dataFeaturesNames.Len();
		if (file.Write(&sz,sizeof(UInt32))==false)
			break;
		if (file.Write(indexFeatureNames.GetPtrToObject(0),indexFeatureNames.Len()*sizeof(UInt32))==false)
			break;
		if (file.Write(dataFeaturesNames.GetPtrToObject(0),dataFeaturesNames.Len())==false)
			break;
		return true;
	}
	if (notifier)
		notifier->Error("[%s] -> Unable to write to file total number of records",ObjectName);
	return false;
}
bool GML::ML::IConnector::LoadFeatureNames()
{
	UInt32	sz;
	if (indexFeatureNames.Create(columns.nrFeatures,true)==false)
	{
		if (notifier)
			notifier->Error("[%s] -> Unable to alloc %d records for features name entries !",ObjectName,columns.nrFeatures);
		return false;
	}
	if (file.Read(&sz,sizeof(UInt32))==false)
	{
		if (notifier)
			notifier->Error("[%s] -> Unable to read from file total number of features",ObjectName);
		return false;
	}
	if (dataFeaturesNames.Create(sz,true)==false)
	{
		if (notifier)
			notifier->Error("[%s] -> Unable to alloc %d bytes for features name entries !",ObjectName,sz);
		return false;
	}
	// citesc	
	while (true)
	{
		if (file.Read(indexFeatureNames.GetPtrToObject(0),columns.nrFeatures * sizeof(UInt32))==false)
			break;
		if (file.Read(dataFeaturesNames.GetPtrToObject(0),sz)==false)
			break;
		return true;
	}
	if (notifier)
		notifier->Error("[%s] -> Unable to read from file features name",ObjectName);
	return false;
}
bool GML::ML::IConnector::SaveRecordHashesAndFeatureNames()
{
	if (SaveRecordHashes()==false)
		return false;
	if (SaveFeatureNames()==false)
		return false;
	return true;
}
bool GML::ML::IConnector::LoadRecordHashesAndFeatureNames(CacheHeader *h)
{
	if (h==NULL)
	{
		if (notifier)
			notifier->Error("[%s] -> NULL parameter for LoadRecordHashesAndFeatureNames function !",ObjectName);
		return false;
	}
	// record hash
	if (h->StoreFlags & GML::ML::ConnectorFlags::STORE_HASH)
	{
		if (StoreRecordHash)
		{
			if (LoadRecordHashes()==false)
				return false;
		} else {
			if (SkipRecordHashes()==false)
				return false;;
		}
	} else {
		StoreRecordHash = false;
	}
	// record weights
	if (h->StoreFlags & GML::ML::ConnectorFlags::STORE_RECORD_WEIGHT)
	{
		StoreRecordWeightMode = (h->StoreFlags >> 3) & 7;
		if (StoreRecordWeight)
		{			
			if (LoadRecordWeights()==false)
				return false;
		} else {
			if (SkipRecordWeights()==false)
				return false;;
		}
	} else {
		StoreRecordWeightMode = 0;
		StoreRecordWeight = false;
	}	
	// numele de la date
	if (h->StoreFlags & GML::ML::ConnectorFlags::STORE_FEATURE_NAME)
	{
		if (StoreFeaturesName)
		{
			if (LoadFeatureNames()==false)
				return false;
		}
	} else {
		StoreFeaturesName = false;
	}
	return true;
}
bool GML::ML::IConnector::UpdateRecordExtraData(GML::ML::MLRecord &record,UInt32 index,UInt32 recordMask)
{
	if (recordMask & GML::ML::ConnectorFlags::STORE_HASH)
	{
		if (StoreRecordHash)
			record.Hash.Copy(*Hashes.GetPtrToObject(index));
		else
			record.Hash.Reset();
	}
	if (recordMask & GML::ML::ConnectorFlags::STORE_RECORD_WEIGHT)
	{
		if (StoreRecordWeight)
			return GetRecordWeight(index,record.Weight);
		else
			record.Weight = 0;
	}
	return true;
}