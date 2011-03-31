#include "DB_MySQL.h"

//FIXME: make sure we have null terminated strings
//FIXME: check strcpy
//FIXME: check all malloc statements
//FIXME: handle selects with LIMIT
//FIXME: remove static allocs

DB_MySQL::DB_MySQL()
{
	ObjectName = "MySQLDatabase";
	this->conn = NULL;
	memset(&this->DBConnStr, 0, sizeof(this->DBConnStr));
	memset(&this->DBResBuff, 0, sizeof(this->DBResBuff));

	LinkPropertyToString("Server", this->DBConnStr.Server, "localhost", "MySQL server address");
	LinkPropertyToString("Database", this->DBConnStr.Database, "", "Database name");
	LinkPropertyToString("Username", this->DBConnStr.Username, "", "Username used for connection");
	LinkPropertyToString("Password", this->DBConnStr.Password, "", "Password used for connection");
	LinkPropertyToUInt32("Port", this->DBConnStr.Port, 3306, "Port for MySQL server");
}

DB_MySQL::~DB_MySQL()
{
	this->Disconnect();
	memset(&this->DBConnStr, 0, sizeof(this->DBConnStr));
}

bool DB_MySQL::OnInit()
{
	return true;
}

bool DB_MySQL::Connect()
{
	if((this->conn = mysql_init(NULL)) == NULL)
		return this->NotifyError();

	if(!mysql_real_connect(this->conn, 
		                    this->DBConnStr.Server, 
							this->DBConnStr.Username, 
							this->DBConnStr.Password, 
							this->DBConnStr.Database, 
							this->DBConnStr.Port, 
							NULL, 0)) 
	{
		this->Disconnect();
		return this->NotifyError();
	}
	return true;
}

bool DB_MySQL::Disconnect()
{
	if(this->DBResBuff.res != NULL)
	{
		mysql_free_result(this->DBResBuff.res);
		this->DBResBuff.res = NULL;
	}
	if(this->DBResBuff.QueryStr != NULL)
	{
		free(DBResBuff.QueryStr);
		memset(&this->DBResBuff, 0, sizeof(this->DBResBuff));
	}
	if(this->conn != NULL)
	{
		mysql_close(this->conn);
		this->conn = NULL;
	}
	return true;
}

UInt32 DB_MySQL::GetRowCount(char *Statement)
{
	MYSQL_RES *Res;
	MYSQL_ROW Row;
	char QueryStr[1024];
	UInt32 RetValue = 0;
	
	memset(QueryStr, 0, sizeof(QueryStr));
	sprintf_s(QueryStr, "select SQL_CALC_FOUND_ROWS %s LIMIT 1", Statement);
	
	if(mysql_query(this->conn, QueryStr)) {
		this->NotifyError();
		return 0;
	}
	Res = mysql_use_result(this->conn);
	mysql_free_result(Res);

	memset(QueryStr, 0, sizeof(QueryStr));
	sprintf_s(QueryStr, "SELECT FOUND_ROWS()");

	if(mysql_query(this->conn, QueryStr)) {
		this->NotifyError();
		return 0;
	}

	Res = mysql_use_result(this->conn);
	Row = mysql_fetch_row(Res);
	RetValue = atoi(Row[0]);
	mysql_free_result(Res);
	
	return RetValue;
}

bool DB_MySQL::CheckCursorPos(char *Statement)
{
	char QueryStr[1024];

	if(this->conn == NULL)
		return this->NotifyError("Database connection not active. Please connect to continue");

	if((Statement != NULL) && (Statement != ""))
	{
		if(this->DBResBuff.QueryStr != NULL)
			free(this->DBResBuff.QueryStr);
		if((this->DBResBuff.QueryStr = (char*)malloc(strlen(Statement)+sizeof(char))) == NULL)
			return this->NotifyError("Could not malloc for res query");
		strcpy_s(this->DBResBuff.QueryStr, sizeof(QueryStr), Statement);
		this->StripQ(this->DBResBuff.QueryStr, "select");
	}

	if((this->DBResBuff.res == NULL) 
		|| (this->DBResBuff.pos_curr > (this->DBResBuff.lim_inf + this->DBResBuff.window_size)) 
		|| (this->DBResBuff.pos_curr < this->DBResBuff.lim_inf))
	{
		if(this->DBResBuff.pos_max == 0) 
		{
			this->DBResBuff.pos_max = this->GetRowCount(this->DBResBuff.QueryStr);
			if(this->DBResBuff.pos_max == 0)
			{
				return false;
			}
			this->DBResBuff.lim_inf = 0; //REMOVE
			this->DBResBuff.window_size = this->DBResBuff.pos_max; //REMOVE
		}
		
		memset(QueryStr, 0, sizeof(QueryStr));
		sprintf_s(QueryStr, sizeof(QueryStr), "select %s", this->DBResBuff.QueryStr); 

		if(mysql_query(this->conn, QueryStr))
		{
			this->NotifyError();
			return false;
		}

		if((this->DBResBuff.res = mysql_store_result(this->conn)) == NULL)
		{
			this->NotifyError();
			return false;
		}

		if(this->DBResBuff.field_num == 0)
		{
			this->DBResBuff.field_num = mysql_num_fields(this->DBResBuff.res);
		}
	}
	return true;
}

UInt32 DB_MySQL::Select (char* Statement)
{
	if(this->notifier == NULL)
		return 0;

	if(!this->CheckCursorPos(Statement))
		return 0;

	if(!this->SetDataType())
		return 0;

	return this->DBResBuff.pos_max;
}

UInt32 DB_MySQL::SqlSelect (char* What, char* Where, char* From)
{
	char QueryStr[1024] = "";
	if((What == NULL) || (What == ""))
	{
		this->NotifyError("Must provide what information to select");
	}
	if((From == NULL) || (From == ""))
	{
		this->NotifyError("Must provide a table name for statement");
		return 0;
	}
	if((Where == NULL) || (Where == ""))
		sprintf_s(QueryStr, sizeof(QueryStr), "select %s from %s", What, From);
	else
		sprintf_s(QueryStr, sizeof(QueryStr), "select %s from %s where %s", What, From, Where);

	return this->Select(QueryStr);
}

bool DB_MySQL::SetDataType()
{
	MYSQL_FIELD *fields;

	if(this->DBResBuff.field_num > 0)
	{
		this->DBResBuff.ColumnNames = new char*[this->DBResBuff.field_num];
		this->DBResBuff.ColumnTypes = new UInt32[this->DBResBuff.field_num];
		if((this->DBResBuff.ColumnNames == NULL) || (this->DBResBuff.ColumnTypes == NULL))
			return this->NotifyError("Could not alloc memory for column data");
	}

	fields = mysql_fetch_fields(this->DBResBuff.res);
	for(UInt32 i = 0; i < this->DBResBuff.field_num; i++)
	{
		if((this->DBResBuff.ColumnNames[i] = (char*)malloc(fields[i].name_length * sizeof(char) + 1)) == NULL)
			return this->NotifyError("Could not malloc memory for column names");
		strcpy_s(this->DBResBuff.ColumnNames[i], (fields[i].name_length * sizeof(char) + 1), fields[i].name);
		switch(fields[i].type)
		{
		case MYSQL_TYPE_DECIMAL:
		case MYSQL_TYPE_TINY:  this->DBResBuff.ColumnTypes[i] = GML::DB::UINT8VAL; break;
		case MYSQL_TYPE_SHORT: this->DBResBuff.ColumnTypes[i] = GML::DB::UINT16VAL; break; 
		case MYSQL_TYPE_LONG:  this->DBResBuff.ColumnTypes[i] = GML::DB::UINT32VAL; break; 
		case MYSQL_TYPE_FLOAT: this->DBResBuff.ColumnTypes[i] = GML::DB::FLOATVAL; break;  
		case MYSQL_TYPE_DOUBLE:this->DBResBuff.ColumnTypes[i] = GML::DB::DOUBLEVAL; break; 
		case MYSQL_TYPE_NULL:  this->DBResBuff.ColumnTypes[i] = GML::DB::NULLVAL; break;  
		case MYSQL_TYPE_TIMESTAMP:this->DBResBuff.ColumnTypes[i] = GML::DB::UINT16VAL; break; 
		case MYSQL_TYPE_LONGLONG: this->DBResBuff.ColumnTypes[i] = GML::DB::UINT64VAL; break; 
		case MYSQL_TYPE_INT24:    this->DBResBuff.ColumnTypes[i] = GML::DB::UINT32VAL; break; 
		case MYSQL_TYPE_DATE:   
		case MYSQL_TYPE_TIME:
		case MYSQL_TYPE_DATETIME:  
		case MYSQL_TYPE_YEAR:
		case MYSQL_TYPE_NEWDATE:
		case MYSQL_TYPE_VARCHAR:
		case MYSQL_TYPE_BIT:
		case MYSQL_TYPE_NEWDECIMAL:
		case MYSQL_TYPE_ENUM:
		case MYSQL_TYPE_SET:
		case MYSQL_TYPE_TINY_BLOB:
		case MYSQL_TYPE_MEDIUM_BLOB:
		case MYSQL_TYPE_LONG_BLOB:
		case MYSQL_TYPE_BLOB:
		case MYSQL_TYPE_VAR_STRING:
		case MYSQL_TYPE_STRING: this->DBResBuff.ColumnTypes[i] = GML::DB::ASCIISTTVAL; break;
		//case MYSQL_TYPE_GEOMETRY:
		//case MAX_NO_FIELD_TYPES:
		default:
			return false; //problem: if a field that needs malloc is beyond this point, strlen will fail in strcpy(dbr.AsciiStrVal, Row[i]) from FetchNextRow
		}
	}
	return true;
}

bool DB_MySQL::FillRow(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr, MYSQL_ROW Row)
{
	for(UInt32 i = 0; i < this->DBResBuff.field_num; i++)
	{
		GML::DB::DBRecord dbr;
		dbr.Type = this->DBResBuff.ColumnTypes[i];
		if((dbr.Name = (char*)malloc(strlen(this->DBResBuff.ColumnNames[i])+1)) == NULL)
			return this->NotifyError("Could not malloc for result name");
		strcpy_s(dbr.Name, strlen(this->DBResBuff.ColumnNames[i])+1, this->DBResBuff.ColumnNames[i]);
		switch(dbr.Type)
		{
		case GML::DB::NULLVAL:   dbr.RawPtrVal = NULL; break;
		case GML::DB::UINT8VAL:  dbr.UInt8Val  = atoi(Row[i]); break;
		case GML::DB::UINT16VAL: dbr.UInt16Val = atoi(Row[i]); break;
		case GML::DB::UINT32VAL: dbr.UInt32Val = atoi(Row[i]); break;
		case GML::DB::UINT64VAL: dbr.UInt64Val = atoi(Row[i]); break;
		case GML::DB::FLOATVAL:  dbr.FloatVal  = (float)atof(Row[i]); break; //Column type is float so there is no data loss
		case GML::DB::DOUBLEVAL: dbr.DoubleVal = atof(Row[i]); break;
		case GML::DB::RAWPTRVAL: dbr.RawPtrVal = NULL; break; //FIXME
		case GML::DB::BYTESVAL:  dbr.BytesVal  = NULL; break; //FIXME
		case GML::DB::ASCIISTTVAL: 
			{
				if((dbr.AsciiStrVal = (char*)malloc(strlen(Row[i]) + 1)) == NULL)
				{
					return this->NotifyError("Could not malloc for result value"); //FIXME: free already alloc space
				}
				strcpy_s(dbr.AsciiStrVal, strlen(Row[i])+1, Row[i]);
			} break;
		case GML::DB::UNICSTRVAL: dbr.UnicStrVal = NULL; break; //FIXME
		//case HASHVAL: dbr.HashVal = 0; break; //FIXME
		default:
			return false;
		}
		VectPtr.Push(dbr);
	}
	return true;
}

bool DB_MySQL::FetchNextRow (GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr)
{
	MYSQL_ROW Row;

	if(VectPtr.Len() != 0)
		this->FreeRow(VectPtr);

	if(!this->CheckCursorPos())
		return false;

	mysql_data_seek(this->DBResBuff.res, (this->DBResBuff.pos_curr - this->DBResBuff.lim_inf)); //FIXME: possibly deprecated
	this->DBResBuff.pos_curr += 1;

	Row = mysql_fetch_row(this->DBResBuff.res);
	if(Row == NULL)
	{
		mysql_free_result(this->DBResBuff.res);
		this->DBResBuff.res = NULL;
		//memset(&this->DBResBuff, 0, sizeof(this->DBResBuff));
		this->DBResBuff.lim_inf = 0;
		this->DBResBuff.pos_curr = 0;
		this->DBResBuff.pos_max = 0;
		return false;
	}

	return FillRow(VectPtr, Row);
}

bool DB_MySQL::FetchRowNr (GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr, UInt32 RowNr)
{
	char Query[1024] = "";
	MYSQL_RES *Res;
	MYSQL_ROW Row;
	bool ret = true;
	char *tmp;

	tmp = strstr(this->DBResBuff.QueryStr, "from");
	if(tmp == NULL)
	{
		return this->NotifyError("Invalid query");
	}

	sprintf_s(Query, sizeof(Query), "select * %s limit %d,1", tmp, RowNr-1);
	if(mysql_query(this->conn, Query)) 
	{
		free(Query);
		return this->NotifyError();
	}

	Res = mysql_use_result(this->conn);

	Row = mysql_fetch_row(Res);
	if(Row == NULL)
	{
		mysql_free_result(Res);
		return false;
	}
	ret = FillRow(VectPtr, Row);
	mysql_free_result(Res);

	return ret;
}

bool DB_MySQL::FreeRow(GML::Utils::GTFVector<GML::DB::DBRecord> &Vect)
{
	for(UInt32 i = 0; i < Vect.Len(); i++)
	{
		switch(Vect[i].Type)
		{
		case GML::DB::BYTESVAL:
		case GML::DB::ASCIISTTVAL:
		case GML::DB::UNICSTRVAL:
		case GML::DB::RAWPTRVAL:
			{
				if(Vect[i].RawPtrVal != NULL)
					free(Vect[i].RawPtrVal);
			}
		}
	}
	return Vect.DeleteAll();
}

bool DB_MySQL::GetColumnInformations(char *columnName,GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr)
{
	return false;
}

bool DB_MySQL::InsertRow (char* Table, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect)
{
	return this->InsertRow(Table, "", Vect);
}

bool DB_MySQL::InsertRow (char* Table, char* Fields, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect)
{
	char Query[1024];

	if((Table == NULL) || (Table == ""))
	{
		this->NotifyError("Table must be specified");
		return false;
	}

	if(Vect.Len() == 0)
	{
		this->NotifyError("No data to insert");
		return false;
	}

	memset(Query, 0, sizeof(Query));
	sprintf_s(Query, sizeof(Query), "insert into %s(%s) values(", Table, Fields);
	
	char to_add[3] = ", ";

	for(UInt32 i = 0; i < Vect.Len(); i++)
	{
		if(i == Vect.Len()-1)
			to_add[0] = ')';
		switch(Vect[i].Type)
		{
		case GML::DB::NULLVAL: sprintf_s(Query, sizeof(Query), "%sNULL%s", Query, to_add); break;
		case GML::DB::UINT8VAL:  sprintf_s(Query, sizeof(Query), "%s'%d'%s", Query, Vect[i].UInt8Val, to_add); break;
		case GML::DB::UINT16VAL: sprintf_s(Query, sizeof(Query), "%s'%d'%s", Query, Vect[i].UInt16Val, to_add); break;
		case GML::DB::UINT32VAL: sprintf_s(Query, sizeof(Query), "%s'%d'%s", Query, Vect[i].UInt32Val, to_add); break;
		case GML::DB::UINT64VAL: sprintf_s(Query, sizeof(Query), "%s'%d'%s", Query, Vect[i].UInt64Val, to_add); break;
		case GML::DB::FLOATVAL:  sprintf_s(Query, sizeof(Query), "%s'%f'%s", Query, Vect[i].FloatVal, to_add); break;
		case GML::DB::DOUBLEVAL: sprintf_s(Query, sizeof(Query), "%s'%f'%s", Query, Vect[i].DoubleVal, to_add); break;
		case GML::DB::ASCIISTTVAL: sprintf_s(Query, sizeof(Query), "%s'%s'%s", Query, Vect[i].AsciiStrVal, to_add); break;
		}
	}

	if(mysql_query(this->conn, Query))
	{
		this->NotifyError();
		return false;
	}

	return true;
}

bool DB_MySQL::Update (char* SqlStatement, GML::Utils::GTFVector<GML::DB::DBRecord> &WhereVals, GML::Utils::GTFVector<GML::DB::DBRecord> &UpdateVals)
{
	return false;
}

bool DB_MySQL::NotifyError(char* Msg)
{
	if(Msg[0]==0)
		this->notifier->Error("%s", (char*)mysql_error(this->conn));
	else
		this->notifier->Error("%s",Msg);
	return false;
}

bool DB_MySQL::StripQ(char* Query, const char* Word)
{
	//strings should be checked before
	UInt32 slen = strlen(Query);
	for(UInt32 i = 0; i < slen; i++)
	{
		if(Query[i] == '\20')
			continue;
		if(Query[i] == Word[0])
		{
			if((i + strlen(Word)) >= slen)
				continue;
			UInt32 j;
			for(j = 1; j < strlen(Word); j++)
			{
				if(Query[i+j] != Word[j])
					goto continue_iter;
			}
			char *tmp;
			if((tmp = (char*)malloc(slen+1)) == NULL)
			{
				this->NotifyError("Could not malloc for query");
				return false;
			}
			strcpy_s(tmp, slen+1, Query);
			memset(Query, 0, slen);
			strncpy_s(Query, slen, tmp, i);
			strcpy_s(&Query[i], slen, &tmp[i+j]);
			return true;
		}
continue_iter:
		1==1;
	}
	return false;
}