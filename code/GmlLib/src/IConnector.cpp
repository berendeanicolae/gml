#include "IConnector.h"


GML::ML::IConnector::IConnector()
{
	notifier = NULL;
	database = NULL;
	conector = NULL;	
	ObjectName = "IConnector";
	columns.indexFeature = NULL;
	ClearColumnIndexes();
	nrRecords = 0;
	dataMemorySize = 0;
	
	LinkPropertyToBool  ("StoreRecordHash",StoreRecordHash,false,"Specify if the connector should store records hash or not");
	LinkPropertyToBool  ("StoreFeatureName",StoreFeaturesName,false,"Specify if the connector should store features name or not");
}
void GML::ML::IConnector::AddDataBaseProperties()
{
	LinkPropertyToString("Query",Query,"SELECT * FROM RecordTable","The query for the select statement");
	LinkPropertyToString("CountQuery",CountQuery,"SELECT COUNT(*) FROM RecordTable","The query for counting the elements in the record set");
	LinkPropertyToUInt32("CachedRecords",CachedRecords,10000,"Number of records to be cached during one SQL query.");
}
void GML::ML::IConnector::AddCacheProperties()
{
	LinkPropertyToString("DataFileName",DataFileName,"","Name of the file that contains data to be loaded");
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
bool GML::ML::IConnector::UpdateColumnInformations(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr)
{
	UInt32				tr,cPoz;
	GML::DB::DBRecord	*rec;


	ClearColumnIndexes();
	for (tr=0;tr<VectPtr.Len();tr++)
	{
		if ((rec=VectPtr.GetPtrToObject(tr))==NULL)
		{
			notifier->Error("[%s] -> Unable to read record #%d",ObjectName,tr);
			return false;
		}
		if (GML::Utils::GString::Equals(rec->Name,"Label",true))
		{			
			// verific ca tipul sa fie unul ok
			if ((rec->Type!=GML::DB::TYPES::DOUBLE) && 
				(rec->Type!=GML::DB::TYPES::BOOLEAN) && 
				(rec->Type!=GML::DB::TYPES::INT16) && 
				(rec->Type!=GML::DB::TYPES::INT32))
			{
				notifier->Error("[%s] -> Invalid type for Label at column #%d. Allowed types: BOOL,INT16,INT32,DOUBLE !",ObjectName,tr);
				return false;
			}
			columns.indexLabel = tr;
		}
		if ((GML::Utils::GString::Equals(rec->Name,"Hash",true))  ||
			(GML::Utils::GString::Equals(rec->Name,"md5f",true)))
		{
			// verific ca tipul sa fie unul ok
			if ((rec->Type!=GML::DB::TYPES::HASH) && 
				(rec->Type!=GML::DB::TYPES::ASCII))
			{
				notifier->Error("[%s] -> Invalid type for Hash at column #%d. Allowed types: ASCIIVAL,HASHVAL !",ObjectName,tr);
				return false;
			}
			columns.indexHash = tr;
		}
		if (GML::Utils::GString::StartsWith(rec->Name,"Ft_",true))
		{
			// verific ca tipul sa fie unul ok
			if ((rec->Type!=GML::DB::TYPES::DOUBLE) && 
				(rec->Type!=GML::DB::TYPES::BOOLEAN) && 
				(rec->Type!=GML::DB::TYPES::INT16) && 
				(rec->Type!=GML::DB::TYPES::INT32))
			{
				notifier->Error("[%s] Invalid type for Feature at column #%d. Allowed types: BOOL,INT16,INT32,DOUBLE !",ObjectName,tr);
				return false;
			}
			columns.nrFeatures++;
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

	// setez si indexii
	for (cPoz=0,tr=0;tr<VectPtr.Len();tr++)
	{
		if ((rec=VectPtr.GetPtrToObject(tr))==NULL)
		{
			notifier->Error("[%s] -> Unable to read record #%d",ObjectName,tr);
			return false;
		}
		if (GML::Utils::GString::StartsWith(rec->Name,"Ft_",true))
		{
			columns.indexFeature[cPoz] = tr;
			if (StoreFeaturesName)
			{
				if (AddColumnName(rec->Name)==false)			
				{
					notifier->Error("[%s] -> Unable to save name for feature #%d (%s)",ObjectName,tr,rec->Name);
					return false;
				}
			}
			cPoz++;			
		}
	}

	return true;
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
		case GML::DB::TYPES::BOOLEAN:
			if (rec->Value.BoolVal)
				value = 1.0;
			else
				value = -1.0;
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
bool GML::ML::IConnector::UpdateColumnInformations(char *QueryStatement)
{
	GML::Utils::GTFVector<GML::DB::DBRecord>	VectPtr;

	if (QueryStatement==NULL)
	{
		notifier->Error("[%s] CountQueryStatement = NULL. Missing query.",ObjectName);
		return false;
	}
	if (database==NULL)
	{
		notifier->Error("[%s] QueryRecordsCount failed. Missing database",ObjectName);
		return false;
	}
	if (database->ExecuteQuery(QueryStatement)==false)
	{
		notifier->Error("[%s] database->ExecuteQuery(%s) failed",ObjectName,QueryStatement);
		return false;
	}
	if (database->GetColumnInformations(VectPtr)==false)
	{
		notifier->Error("[%s] -> Error reading column informations for query [%s]",ObjectName,QueryStatement);
		return false;
	}
	if (UpdateColumnInformations(VectPtr)==false)
		return false;
	return true;
}
bool GML::ML::IConnector::QueryRecordsCount(char *CountQueryStatement,UInt32 &recordsCount)
{
	UInt32										resRows;
	GML::Utils::GTFVector<GML::DB::DBRecord>	VectPtr;
	GML::DB::DBRecord							*rec;

	recordsCount = 0;
	if (CountQueryStatement==NULL)
	{
		notifier->Error("[%s] CountQueryStatement = NULL. Missing query.",ObjectName);
		return false;
	}
	if (database==NULL)
	{
		notifier->Error("[%s] QueryRecordsCount failed. Missing database",ObjectName);
		return false;
	}
	if (database->ExecuteQuery(CountQueryStatement,&resRows)==false)
	{
		notifier->Error("[%s] database->ExecuteQuery(%s) failed",ObjectName,CountQueryStatement);
		return false;
	}
	if (resRows!=1)
	{
		notifier->Error("[%s] database->ExecuteQuery(%s) returns %d rows (it should have returned one row)",ObjectName,CountQueryStatement,resRows);
		return false;
	}
	if (database->FetchNextRow(VectPtr)==false)
	{
		notifier->Error("[%s] database->FetchNextRow for query (%s) failed.",ObjectName,CountQueryStatement);
		return false;
	}
	if (VectPtr.Len()!=1)
	{
		notifier->Error("[%s] database->FetchNextRow for query (%s) should have returrned one value.",ObjectName,CountQueryStatement);
		return false;
	}
	if ((rec = VectPtr.GetPtrToObject(0))==NULL)
	{
		notifier->Error("[%s] Internal error 'VectPtr.GetPtrToObject(0)'",ObjectName);
		return false;
	}
	switch (rec->Type)
	{
		case GML::DB::TYPES::INT32:
			recordsCount = (UInt32)rec->Value.Int32Val;
			break;
		case GML::DB::TYPES::UINT32:
			recordsCount = (UInt32)rec->Value.UInt32Val;
			break;
		case GML::DB::TYPES::UINT64:
			recordsCount = (UInt32)rec->Value.UInt64Val;
			break;
		default:
			notifier->Error("[%s] '%s' returnes an invalid type (non-numeric - %d )",ObjectName,CountQueryStatement,rec->Type);
			return false;
	}
	return true;
}
bool GML::ML::IConnector::Init(GML::Utils::INotifier &_notifier,GML::DB::IDataBase *_database,GML::ML::IConnector *_connector,char *attributeString)
{
	bool										result;

	// daca a fost deja initializat
	if ((database!=NULL) || (conector!=NULL))
	{
		if (notifier)
			notifier->Error("[%s] -> Conector already initilized !",ObjectName);
		return false;
	}

	if ((attributeString!=NULL) && (attributeString[0]!=0))
	{
		if (SetProperty(attributeString)==false)
		{
			notifier->Error("[%s] -> Invalid format for Conector initializations: %s",ObjectName,attributeString);
			return false;
		}
	}

	notifier = &_notifier;
	database = _database;
	conector = _connector;
	ClearColumnIndexes();

	notifier->Info("[%s] -> OnInit()",ObjectName);
	result = OnInit();
	if (result==false)
		notifier->Error("[%s] -> OnInit() returned false",ObjectName);
	return result;
}
bool GML::ML::IConnector::Init(GML::Utils::INotifier &_notifier,GML::DB::IDataBase &_database,char *attributeString)
{	
	return Init(_notifier,&_database,NULL,attributeString);
}
bool GML::ML::IConnector::Init(GML::ML::IConnector &_conector,char *attributeString)
{
	return Init(*(_conector.notifier),NULL,&_conector,attributeString);
}
bool GML::ML::IConnector::Init(GML::Utils::INotifier &_notifier,char *attributeString)
{
	return Init(_notifier,NULL,NULL,attributeString);
}
bool GML::ML::IConnector::Save(char *fileName)
{
	if (notifier)
		notifier->Error("[%s] Save function not implemented ",ObjectName);
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
		notifier->Error("[%s] Connection to another connector is not suported !",ObjectName);
	return false;
}
bool GML::ML::IConnector::OnInitConnectionToCache()
{
	if (notifier==NULL)
	{
		DEBUGMSG("[%s] Notifier should be set first before executing this function !",ObjectName);
		return false;
	}
	if (DataFileName.Len()==0)
	{
		notifier->Error("[%s] Property 'DataFileName' was not set !",ObjectName);
		return false;
	}
	return Load(DataFileName.GetText());
}
bool GML::ML::IConnector::OnInit()
{
	GML::Utils::GString		temp;
	dataMemorySize = 0;
	if (notifier==NULL)
	{
		DEBUGMSG("[%s] Notifier should be set first before executing this function !",ObjectName);
		return false;
	}
	ClearColumnIndexes();
	while (true)
	{
		// daca exista o baza de date , incerc conectarea la ea
		if (database!=NULL)
		{
			if (OnInitConnectionToDataBase()==false)
				return false;
			break;
		}
		// daca exista un connector ma conectez la el
		if (conector!=NULL)
		{
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

	}
	return true;
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
		notifier->Info("[%s] -> Loading cache to: %s",ObjectName,fileName);
	if (headerSize<sizeof(CacheHeader))
	{
		if (notifier)
			notifier->Error("[%s] -> Header should be at least %d bytes!",ObjectName,sizeof(CacheHeader));
		return false;
	}
	if (file.OpenRead(fileName)==false)
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
	MEMSET(header,0,headerSize);
	if (GML::Utils::GString::Set(header->MagicName,sigName,31)==false)
	{
		if (notifier)
			notifier->Error("[%s] -> SigName should be at least 31 bytes",ObjectName);
		return false;
	}
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
	UInt32	cPoz;
	cPoz = file.GetFilePos();
	cPoz += nrRecords*sizeof(GML::DB::RecordHash);
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
			notifier->Error("[%s] -> Unable to alloc %d records for hash entries !",ObjectName,nrRecords);
		return false;
	}
	if (file.Read(Hashes.GetPtrToObject(0),nrRecords*sizeof(GML::DB::RecordHash))==false)
	{
		if (notifier)
			notifier->Error("[%s] -> Unable to read from file total number of records",ObjectName);
		return false;
	}
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