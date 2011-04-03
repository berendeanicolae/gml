#include "GDTMySql.h"


GDTMySQL::GDTMySQL()
{
	ObjectName = "MySQL";
	conn = NULL;
	result = NULL;
	columnNames = NULL;

	LinkPropertyToString("Server", Server, "127.0.0.1", "MySQL server address");
	LinkPropertyToString("Database", Database, "", "Database name");
	LinkPropertyToString("Username", Username, "", "Username used for connection");
	LinkPropertyToString("Password", Password, "", "Password used for connection");
	LinkPropertyToUInt32("Port", Port, 3306, "Port for MySQL server");
}
bool   GDTMySQL::MySQLError(char *text)
{
	if (conn==NULL)
		notifier->Error("[%s] MySQL Error (%s) -> [No connection]",ObjectName,text);
	else
		notifier->Error("[%s] MySQL Error (%s) -> %s", ObjectName, text,(char*)mysql_error(this->conn));
	return false;
}
bool   GDTMySQL::OnInit()
{
	Disconnect();
	return true;
}
bool   GDTMySQL::Disconnect()
{
	if (result!=NULL)
	{
		 mysql_free_result(result);
		 result = NULL;
	}
	if (conn!=NULL)
	{
		mysql_close(conn);
		conn = NULL;
	}
	return true;
}
bool   GDTMySQL::Connect ()
{
	Disconnect();
	if ((conn = mysql_init(NULL))==NULL)
	{
		notifier->Error("[%s] Unable to init mysql",ObjectName);
		return false;
	}
	if(!mysql_real_connect(	conn,Server.GetText(),Username.GetText(),Password.GetText(),Database.GetText(),Port,NULL, 0)) 
	{
		MySQLError("Connection error");
		Disconnect();
		return false;
	}
	return true;
}
UInt32 GDTMySQL::SqlSelect (char* What, char* Where, char* From)
{
	notifier->Error("[%s] Not implemented : SqlSelect",ObjectName);
	return 0;
}
bool   GDTMySQL::InsertRow (char* Table, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect)
{
	notifier->Error("[%s] Not implemented : InsertRow",ObjectName);
	return false;
}
bool   GDTMySQL::InsertRow (char* Table, char* Fields, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect)
{
	notifier->Error("[%s] Not implemented : InsertRow",ObjectName);
	return false;
}
bool   GDTMySQL::Update (char* SqlStatement, GML::Utils::GTFVector<GML::DB::DBRecord> &WhereVals, GML::Utils::GTFVector<GML::DB::DBRecord> &UpdateVals)
{
	notifier->Error("[%s] Not implemented : Update",ObjectName);
	return false;
}
bool   GDTMySQL::FetchRowNr (GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr, UInt32 RowNr)
{
	notifier->Error("[%s] Not implemented : FetchRowNr",ObjectName);
	return false;
}
bool   GDTMySQL::FreeRow(GML::Utils::GTFVector<GML::DB::DBRecord> &Vect)
{	
	return true;
}
UInt32 GDTMySQL::Select (char* Statement)
{
	if (columnNames!=NULL)
	{
		delete columnNames;
		columnNames = NULL;
	}
	if (conn==NULL)
	{
		MySQLError("Missing DB connection");
		return 0;
	}
	if (result!=NULL)
	{
		 mysql_free_result(result);
		 result = NULL;
	}
	if (mysql_query(conn,Statement)!=0)
	{
		notifier->Error("[%s] Error processing statement : %s",ObjectName,Statement);
		MySQLError("Error on Select");
		return 0;
	}
	if ((result = mysql_store_result(conn))==NULL)
	{
		MySQLError("Error on StoreResult");
		return 0;
	}
	return (UInt32)mysql_num_rows(result);
}
bool   GDTMySQL::GetColumnInformations(char *tableName,GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr)
{
	GML::Utils::GString		tempStr;	
	MYSQL_FIELD				*field;
	int						fieldsCount;
	GML::DB::DBRecord		rec;

	VectPtr.DeleteAll();
	if (tempStr.SetFormated("SELECT * from %s LIMIT 1",tableName)==false)
	{
		notifier->Error("[%s] Unable to create query ...",ObjectName);
		return false;
	}
	if (Select(tempStr.GetText())==0)
		return false;
	fieldsCount = mysql_num_fields(result);
	if (fieldsCount<=0)
	{
		notifier->Error("[%s] Invalid number of columns for %s (%d)",ObjectName,tempStr.GetText(),fieldsCount);
		return false;		
	}
	if ((columnNames = new GML::Utils::GString[fieldsCount])==NULL)
	{
		notifier->Error("[%s] Unable to alloc %d columns for %s ",ObjectName,fieldsCount,tempStr.GetText());
		return false;		
	}
	for (int tr=0;tr<fieldsCount;tr++)
	{
		if ((field = mysql_fetch_field_direct(result, tr))==NULL)
			return MySQLError("mysql_fetch_field_direct failed !");
		columnNames[tr].Set(field->name);
		rec.Name = columnNames[tr].GetText();
		switch (field->type)
		{
			case MYSQL_TYPE_DOUBLE:
				rec.Type = GML::DB::DOUBLEVAL;
				break;
			case MYSQL_TYPE_BIT:
				rec.Type = GML::DB::BOOLVAL;
				break;
			default:
				notifier->Error("[%s] Unknwon column type (%d) for column %s",ObjectName,field->type,field->name);
				return false;
		}
		if (VectPtr.PushByRef(rec)==false)
		{
			notifier->Error("[%s] Internal error (VectPtr.PushByRef)",ObjectName);
			return false;
		}
	}
	return true;
}
bool   GDTMySQL::FetchNextRow (GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr)
{
	MYSQL_FIELD				*field;
	int						fieldsCount;
	GML::DB::DBRecord		rec;
	MYSQL_ROW				row;

	VectPtr.DeleteAll();
	fieldsCount = mysql_num_fields(result);
	row = mysql_fetch_row(result);

	for (int tr=0;tr<fieldsCount;tr++)
	{
		if ((field = mysql_fetch_field_direct(result, tr))==NULL)
			return MySQLError("mysql_fetch_field_direct failed !");
		rec.Name = field->name;
		switch (field->type)
		{
			case MYSQL_TYPE_DOUBLE:
				rec.Type = GML::DB::DOUBLEVAL;
				if (GML::Utils::GString::ConvertToDouble(row[tr],&rec.DoubleVal)==false)
				{
					notifier->Error("[%s] Invalid conversion to double (%s) on column %s",ObjectName,row[tr],field->name);
					return false;
				}
				break;
			case MYSQL_TYPE_BIT:
				rec.Type = GML::DB::BOOLVAL;
				if (row[tr][0]!=0)
					rec.BoolVal = true;
				else
					rec.BoolVal = false;
				break;
			default:
				notifier->Error("[%s] Unknwon column type (%d) for column %s",ObjectName,field->type,field->name);
				return false;
		}
		if (VectPtr.PushByRef(rec)==false)
		{
			notifier->Error("[%s] Internal error (VectPtr.PushByRef)",ObjectName);
			return false;
		}
	}
	return true;
}