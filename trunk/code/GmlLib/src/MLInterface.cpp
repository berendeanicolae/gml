#include "MLInterface.h"


GML::ML::IConnector::IConnector()
{
	notifier = NULL;
	database = NULL;
	conector = NULL;	
	ObjectName = "IConnector";
	columns.indexFeature = NULL;
	columns.featName = NULL;
	ClearColumnIndexes();
	
	LinkPropertyToBool  ("StoreRecordHash",StoreRecordHash,false,"Specify if the connector should store records hash or not");
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
	if (columns.featName!=NULL)
		delete columns.featName;
	columns.indexFeature = NULL;
	columns.featName = NULL;
	columns.nrFeatures = 0;
	columns.indexLabel = -1;
	columns.indexHash = -1;
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
			if ((rec->Type!=GML::DB::DOUBLEVAL) && 
				(rec->Type!=GML::DB::BOOLVAL) && 
				(rec->Type!=GML::DB::INT16VAL) && 
				(rec->Type!=GML::DB::INT32VAL))
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
			if ((rec->Type!=GML::DB::HASHVAL) && 
				(rec->Type!=GML::DB::ASCIISTTVAL))
			{
				notifier->Error("[%s] -> Invalid type for Hash at column #%d. Allowed types: ASCIIVAL,HASHVAL !",ObjectName,tr);
				return false;
			}
			columns.indexHash = tr;
		}
		if (GML::Utils::GString::StartsWith(rec->Name,"Ft_",true))
		{
			// verific ca tipul sa fie unul ok
			if ((rec->Type!=GML::DB::DOUBLEVAL) && 
				(rec->Type!=GML::DB::BOOLVAL) && 
				(rec->Type!=GML::DB::INT16VAL) && 
				(rec->Type!=GML::DB::INT32VAL))
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
	if ((columns.featName = new GML::Utils::GString[columns.nrFeatures])==NULL)
	{
		notifier->Error("[%s] -> Unable to alloc %d features names",ObjectName,columns.nrFeatures);
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
			if (columns.featName[cPoz].Set(rec->Name)==false)
			{
				notifier->Error("[%s] -> Unable to save name for feature #%d (%s)",ObjectName,tr,rec->Name);
				return false;
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
		case GML::DB::DOUBLEVAL:
			value = rec->DoubleVal;
			break;
		case GML::DB::INT8VAL:
			value = (double)rec->Int8Val;
			break;
		case GML::DB::INT16VAL:
			value = (double)rec->Int16Val;
			break;
		case GML::DB::INT32VAL:
			value = (double)rec->Int32Val;
			break;
		case GML::DB::BOOLVAL:
			if (rec->BoolVal)
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
		case GML::DB::ASCIISTTVAL:
			if (recHash.CreateFromText(rec->AsciiStrVal)==false)
			{
				notifier->Error("[%s] -> Unable to convert '%s' to a valid hash !",ObjectName,rec->AsciiStrVal);
				return false;
			}
			break;
		case GML::DB::HASHVAL:
			recHash.Copy(rec->Hash);
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
		case GML::DB::INT32VAL:
			recordsCount = (UInt32)rec->Int32Val;
			break;
		case GML::DB::UINT32VAL:
			recordsCount = (UInt32)rec->UInt32Val;
			break;
		case GML::DB::UINT64VAL:
			recordsCount = (UInt32)rec->UInt64Val;
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
	if (notifier==NULL)
	{
		DEBUGMSG("[%s] Notifier should be set first before executing this function !",ObjectName);
		return false;
	}
	// daca exista o baza de date , incerc conectarea la ea
	if (database!=NULL)
		return OnInitConnectionToDataBase();
	// daca exista un connector ma conectez la el
	if (conector!=NULL)
		return OnInitConnectionToConnector();
	// altfel incerc si cu cache-ul
	return OnInitConnectionToCache();
}
bool GML::ML::IConnector::GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index)
{
	if (notifier)
		notifier->Error("[%s] GetRecordHash not implemented !",ObjectName);
	return false;
}
bool GML::ML::IConnector::GetFeatureName(GML::Utils::GString &str,UInt32 index)
{
	char *name = "";
	if (index>=GetFeatureCount())
	{
		notifier->Error("[%s] -> Invalid index (%d) for feature name. Should be within [0..%d]",ObjectName,index,GetFeatureCount()-1);
		return false;
	}
	if (columns.featName!=NULL)
	{
		name = columns.featName[index].GetText();
		if (name==NULL)
			name = "";
	}
	if (str.Set(name)==false)
	{
		notifier->Error("[%s] -> Unable to set feature name : %s",ObjectName,name);
		return false;
	}
	return true;
}
