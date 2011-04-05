#include "MLInterface.h"


GML::ML::IConnector::IConnector()
{
	notifier = NULL;
	database = NULL;
	conector = NULL;	
	ObjectName = "IConnector";
	columns.indexFeature = NULL;
	ClearColumnIndexes();
	
	LinkPropertyToString("Query",Query,"SELECT * FROM RecordTable","The query for the select statement");
	LinkPropertyToString("CountQuery",CountQuery,"SELECT COUNT(*) FROM RecordTable","The query for counting the elements in the record set");
	LinkPropertyToString("DataFileName",DataFileName,"","Name of the file that contains data to be loaded");
	LinkPropertyToUInt32("CachedRecords",CachedRecords,10000,"Number of records to be cached during one SQL query.");
}
void GML::ML::IConnector::ClearColumnIndexes()
{
	if (columns.indexFeature!=NULL)
		delete columns.indexFeature;
	columns.indexFeature = NULL;
	columns.nrFeatures = 0;
	columns.indexLabel = -1;
	columns.indexHash = -1;
}
bool GML::ML::IConnector::UpdateColumnInformations(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr)
{
	UInt32				tr,value;
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
		if (GML::Utils::GString::Equals(rec->Name,"Hash",true))
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
		if (GML::Utils::GString::StartsWith(rec->Name,"Feat_",true))
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
	// pun o valoare default
	for (tr=0;tr<columns.nrFeatures;tr++)
	{
		columns.indexFeature[tr] = -1;
	}
	// setez si indexii
	for (tr=0;tr<VectPtr.Len();tr++)
	{
		if ((rec=VectPtr.GetPtrToObject(tr))==NULL)
		{
			notifier->Error("[%s] -> Unable to read record #%d",ObjectName,tr);
			return false;
		}
		if (GML::Utils::GString::StartsWith(rec->Name,"Feat_",true))
		{
			if (GML::Utils::GString::ConvertToUInt32(&rec->Name[5],&value,10)==false)
			{
				notifier->Error("[%s] -> Invalid numeric format on column: %s",ObjectName,rec->Name);
				return false;
			}
			if (value>=columns.nrFeatures)
			{
				notifier->Error("[%s] -> Out of the bounds feature ([0..%d)) -> %s",ObjectName,columns.nrFeatures,rec->Name);
				return false;
			}
			columns.indexFeature[value] = tr;
		}
	}
	// verific sa fi fost setati toti featureii
	for (tr=0;tr<columns.nrFeatures;tr++)
	{
		if (columns.indexFeature[tr]==-1)
		{
			notifier->Error("[%s] -> Feature #d is not present !",ObjectName,tr);
			return false;
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
bool GML::ML::IConnector::Init(GML::Utils::INotifier &_notifier,GML::DB::IDataBase &_database,char *attributeString)
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
	database = &_database;
	conector = NULL;

	notifier->Info("[%s] -> OnInit()",ObjectName);
	result = OnInit();
	if (result==false)
		notifier->Error("[%s] -> OnInit() returned false",ObjectName);
	return result;
}
bool GML::ML::IConnector::Init(GML::ML::IConnector &_conector,char *attributeString)
{
	bool result;
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

	conector = &_conector;
	notifier = conector->notifier;
	database = NULL;
	ClearColumnIndexes();

	notifier->Info("[%s] -> OnInit()",ObjectName);
	result = OnInit();
	if (result==false)
		notifier->Error("[%s] -> OnInit() returned false",ObjectName);
	return result;
}
bool GML::ML::IConnector::Init(GML::Utils::INotifier &Notifier,char *attributeString)
{
	bool result;
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

	notifier = &Notifier;
	database = NULL;
	conector = NULL;
	ClearColumnIndexes();

	if (Load(DataFileName.GetText())==false)
	{
		notifier->Error("[%s] Unable to load data from %s",ObjectName,DataFileName.GetText());
		return true;
	}

	notifier->Info("[%s] -> OnInit()",ObjectName);
	result = OnInit();
	if (result==false)
		notifier->Error("[%s] -> OnInit() returned false",ObjectName);
	return result;
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