#include "GDTMySql.h"


GDTMySQL::GDTMySQL()
{
	ObjectName = "MySQL";
	conn = NULL;
	result = NULL;
	
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

bool   GDTMySQL::ExecuteQuery (char* Statement,UInt32 *rowsCount)
{
	if (conn==NULL)
		return MySQLError("Missing DB connection");

	if (result!=NULL)
	{
		 mysql_free_result(result);
		 result = NULL;
	}
	if (mysql_query(conn,Statement)!=0)
	{
		notifier->Error("[%s] Error processing statement : %s",ObjectName,Statement);
		return MySQLError("Error on Select");
	}
	if ((result = mysql_store_result(conn))==NULL)
		return MySQLError("Error on StoreResult");
	if (rowsCount)
		(*rowsCount) = (UInt32)mysql_num_rows(result);

	return true;
}
bool   GDTMySQL::GetColumnInformations(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr)
{
	GML::Utils::GString		tempStr;	
	MYSQL_FIELD				*field;
	int						fieldsCount;
	GML::DB::DBRecord		rec;

	VectPtr.DeleteAll();
	if ((result==NULL) || (conn==NULL))
	{
		notifier->Error("[%s] -> GetColumnInformations should be used after an ExecuteQuery command !");
		return false;
	}	
	fieldsCount = mysql_num_fields(result);
	if (fieldsCount<=0)
	{
		notifier->Error("[%s] Invalid number of columns for %s (%d)",ObjectName,tempStr.GetText(),fieldsCount);
		return false;		
	}
	for (int tr=0;tr<fieldsCount;tr++)
	{
		if ((field = mysql_fetch_field_direct(result, tr))==NULL)
			return MySQLError("mysql_fetch_field_direct failed !");
		
		rec.Name = field->name;
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

	if ((result==NULL) || (conn==NULL))
	{
		notifier->Error("[%s] -> FetchNextRow should be used after an ExecuteQuery command !");
		return false;
	}	

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
					notifier->Error("[%s] -> Invalid conversion to double (%s) on column %s",ObjectName,row[tr],field->name);
					return false;
				}
				break;
			case MYSQL_TYPE_LONGLONG:
				rec.Type = GML::DB::UINT64VAL;
				if (GML::Utils::GString::ConvertToUInt64(row[tr],&rec.UInt64Val)==false)
				{
					notifier->Error("[%s] -> Invalid conversion to UInt64 (%s) on column %s",ObjectName,row[tr],field->name);
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
				notifier->Error("[%s] -> Unknwon column type (%d) for column %s",ObjectName,field->type,field->name);
				return false;
		}
		if (VectPtr.PushByRef(rec)==false)
		{
			notifier->Error("[%s] -> Internal error (VectPtr.PushByRef)",ObjectName);
			return false;
		}
	}
	return true;
}