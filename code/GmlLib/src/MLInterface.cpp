#include "MLInterface.h"


GML::ML::IConnector::IConnector()
{
	notifier = NULL;
	database = NULL;
	conector = NULL;	
	columns.indexFeature = NULL;
	ClearColumnIndexes();
	LinkPropertyToString("Table",TableName,"RecordTable","Name of the table from the database that will be used");
	LinkPropertyToString("SelectQuery",SelectQuery,"*","The query for the select statement");
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
			notifier->Error("Unable to read record #%d",tr);
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
				notifier->Error("Invalid type for Label at column #%d. Allowed types: BOOL,INT16,INT32,DOUBLE !",tr);
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
				notifier->Error("Invalid type for Hash at column #%d. Allowed types: ASCIIVAL,HASHVAL !",tr);
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
				notifier->Error("Invalid type for Feature at column #%d. Allowed types: BOOL,INT16,INT32,DOUBLE !",tr);
				return false;
			}
			columns.nrFeatures++;
		}
	}
	if (columns.nrFeatures==0)
	{
		notifier->Error("Missing Feature from the column header !");
		return false;
	}
	if (columns.indexLabel==-1)
	{
		notifier->Error("Missing Label from the column header !");
		return false;
	}
	// aloc indexii
	if ((columns.indexFeature = new Int32[columns.nrFeatures])==NULL)
	{
		notifier->Error("Unable to alloc %d features indexes ",columns.nrFeatures);
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
			notifier->Error("Unable to read record #%d",tr);
			return false;
		}
		if (GML::Utils::GString::StartsWith(rec->Name,"Feat_",true))
		{
			if (GML::Utils::GString::ConvertToUInt32(&rec->Name[5],&value)==false)
			{
				notifier->Error("Invalid numeric format on column: %s",rec->Name);
				return false;
			}
			if (value>=columns.nrFeatures)
			{
				notifier->Error("Out of the bounds feature ([0..%d)) -> %s",columns.nrFeatures,rec->Name);
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
			notifier->Error("Feature #d is not present !",tr);
			return false;
		}
	}

	return true;
}
bool GML::ML::IConnector::UpdateDoubleValue(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr,Int32 index,double *value)
{
	GML::DB::DBRecord	*rec;

	if ((rec = VectPtr.GetPtrToObject(index))==NULL)
	{
		notifier->Error("Unable to read record with index #%d",index);
		return false;
	}
	switch (rec->Type)
	{
		case GML::DB::DOUBLEVAL:
			(*value) = rec->DoubleVal;
			break;
		case GML::DB::INT8VAL:
			(*value) = (double)rec->Int8Val;
			break;
		case GML::DB::INT16VAL:
			(*value) = (double)rec->Int16Val;
			break;
		case GML::DB::INT32VAL:
			(*value) = (double)rec->Int32Val;
			break;
		case GML::DB::BOOLVAL:
			(*value) = (double)rec->BoolVal;
			break;
		default:
			notifier->Error("Unable to convert column from index %d to double !",index);
			return false;
	}
	return true;
}
bool GML::ML::IConnector::Init(GML::Utils::INotifier &_notifier,GML::DB::IDataBase &_database,char *attributeString)
{	
	GML::Utils::GTFVector<GML::DB::DBRecord>	VectPtr;

	// daca a fost deja initializat
	if ((database!=NULL) || (conector!=NULL))
	{
		if (notifier)
			notifier->Error("Conector already initilized !");
		return false;
	}

	if ((attributeString!=NULL) && (attributeString[0]!=0))
	{
		if (SetProperty(attributeString)==false)
		{
			notifier->Error("Invalid format for Conector initializations: %s",attributeString);
			return false;
		}
	}

	notifier = &_notifier;
	database = &_database;
	conector = NULL;

	if (database->GetColumnInformations(TableName.GetText(),VectPtr)==false)
	{
		notifier->Error("Error reading column informations from DBTable: [%s]",TableName.GetText());
		return false;
	}
	if (UpdateColumnInformations(VectPtr)==false)
		return false;

	return OnInit();
}
bool GML::ML::IConnector::Init(GML::ML::IConnector &_conector,char *attributeString)
{
	// daca a fost deja initializat
	if ((database!=NULL) || (conector!=NULL))
	{
		if (notifier)
			notifier->Error("Conector already initilized !");
		return false;
	}

	if ((attributeString!=NULL) && (attributeString[0]!=0))
	{
		if (SetProperty(attributeString)==false)
		{
			notifier->Error("Invalid format for Conector initializations: %s",attributeString);
			return false;
		}
	}

	conector = &_conector;
	notifier = conector->notifier;
	database = NULL;
	ClearColumnIndexes();

	return OnInit();
}