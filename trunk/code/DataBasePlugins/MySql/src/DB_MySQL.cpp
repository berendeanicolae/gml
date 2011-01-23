#include "DB_MySQL.h"

//FIXME: make sure we have null terminated strings
//FIXME: check strcpy
//FIXME: check all malloc statements
//FIXME: handle selects with LIMIT
//FIXME: remove static allocs

DB_MySQL::DB_MySQL()
{
	this->conn = NULL;
	memset(&this->DBConnStr, 0, sizeof(this->DBConnStr));
	memset(&this->DBResBuff, 0, sizeof(this->DBResBuff));
}

DB_MySQL::~DB_MySQL()
{
	this->Disconnect();
	if(this->DBConnStr.Server != NULL)
		free(this->DBConnStr.Server);
	if(this->DBConnStr.Database != NULL)
		free(this->DBConnStr.Database);
	if(this->DBConnStr.Username != NULL)
		free(this->DBConnStr.Username);
	if(this->DBConnStr.Password != NULL)
		free(this->DBConnStr.Password);
	memset(&this->DBConnStr, 0, sizeof(this->DBConnStr));
}

bool DB_MySQL::Init (INotifier &notifier, char* Server, char* Database, char* Username, char* Password, UInt32 Port)
{
	if(this->conn != NULL)
		return false;

	this->notifier = &notifier;
	this->DBConnStr.Port = Port;
	if((this->DBConnStr.Server = (char*)malloc(strlen(Server)+sizeof(char))) == NULL)
			return false;
	if((this->DBConnStr.Database = (char*)malloc(strlen(Database)+sizeof(char))) == NULL)
			return false;
	if((this->DBConnStr.Username = (char*)malloc(strlen(Username)+sizeof(char))) == NULL)
			return false;
	if((this->DBConnStr.Password = (char*)malloc(strlen(Password)+sizeof(char))) == NULL)
			return false;
	strcpy(this->DBConnStr.Server, Server);
	strcpy(this->DBConnStr.Database, Database);
	strcpy(this->DBConnStr.Username, Username);
	strcpy(this->DBConnStr.Password, Password);
	
	return true;
}

bool DB_MySQL::Connect ()
{
	if((this->conn = mysql_init(NULL)) == NULL)
	{
		this->NotifyError();
		return false;
	}

	if (!mysql_real_connect(this->conn, 
		                    this->DBConnStr.Server, 
							this->DBConnStr.Username, 
							this->DBConnStr.Password, 
							this->DBConnStr.Database, 
							this->DBConnStr.Port, 
							NULL, 0)) 
	{
		this->NotifyError();
		this->Disconnect();
		return false;
	}
	return true;
}

bool DB_MySQL::Disconnect ()
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
	{
		this->NotifyError("Database connection not active. Please connect to continue");
		return false;
	}

	if((Statement != NULL) && (Statement != ""))
	{
		if(this->DBResBuff.QueryStr != NULL)
			free(this->DBResBuff.QueryStr);
		this->DBResBuff.QueryStr = (char*)malloc(strlen(Statement)+sizeof(char)); //FIXME check for errors
		strcpy(this->DBResBuff.QueryStr, Statement);
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
		sprintf(QueryStr, "select %s", this->DBResBuff.QueryStr); 

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
		sprintf(QueryStr, "select %s from %s", What, From);
	else
		sprintf(QueryStr, "select %s from %s where %s", What, From, Where);

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
		{
			this->NotifyError("Could not alloc memory for column data");
			return false;
		}
	}

	fields = mysql_fetch_fields(this->DBResBuff.res);
	for(UInt32 i = 0; i < this->DBResBuff.field_num; i++)
	{
		if((this->DBResBuff.ColumnNames[i] = (char*)malloc(fields[i].name_length * sizeof(char) + 1)) == NULL)
		{
			this->NotifyError("Could not malloc memory for column names");
			return false;
		}
		strcpy(this->DBResBuff.ColumnNames[i], fields[i].name);
		switch(fields[i].type)
		{
		case MYSQL_TYPE_DECIMAL:
		case MYSQL_TYPE_TINY:  this->DBResBuff.ColumnTypes[i] = UINT8VAL; break;
		case MYSQL_TYPE_SHORT: this->DBResBuff.ColumnTypes[i] = UINT16VAL; break; 
		case MYSQL_TYPE_LONG:  this->DBResBuff.ColumnTypes[i] = UINT32VAL; break; 
		case MYSQL_TYPE_FLOAT: this->DBResBuff.ColumnTypes[i] = UINT32VAL; break;  //FIXME: float
		case MYSQL_TYPE_DOUBLE:this->DBResBuff.ColumnTypes[i] = UINT32VAL; break;  //FIXME: float
		case MYSQL_TYPE_NULL:  this->DBResBuff.ColumnTypes[i] = NULLVAL; break;  
		case MYSQL_TYPE_TIMESTAMP:this->DBResBuff.ColumnTypes[i] = UINT16VAL; break; 
		case MYSQL_TYPE_LONGLONG: this->DBResBuff.ColumnTypes[i] = UINT64VAL; break; 
		case MYSQL_TYPE_INT24:    this->DBResBuff.ColumnTypes[i] = UINT32VAL; break; 
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
		case MYSQL_TYPE_STRING: this->DBResBuff.ColumnTypes[i] = ASCIISTTVAL; break;
		//case MYSQL_TYPE_GEOMETRY:
		//case MAX_NO_FIELD_TYPES:
		default:
			return false; //problem: if a field that needs malloc is beyond this point, strlen will fail in strcpy(dbr.AsciiStrVal, Row[i]) from FetchNextRow
		}
	}
	return true;
}

bool DB_MySQL::FillRow(DbRecordVect &VectPtr, MYSQL_ROW Row)
{
	for(UInt32 i = 0; i < this->DBResBuff.field_num; i++)
	{
		DbRecord dbr;
		dbr.Type = this->DBResBuff.ColumnTypes[i];
		if((dbr.Name = (char*)malloc(strlen(this->DBResBuff.ColumnNames[i])+1)) == NULL)
		{
			this->NotifyError("Could not malloc for result name");
			return false;
		}
		strcpy(dbr.Name, this->DBResBuff.ColumnNames[i]);
		switch(dbr.Type)
		{
		case NULLVAL:   dbr.RawPtrVal = NULL; break;
		case UINT8VAL:  dbr.UInt8Val  = atoi(Row[i]); break;
		case UINT16VAL: dbr.UInt16Val = atoi(Row[i]); break;
		case UINT32VAL: dbr.UInt32Val = atoi(Row[i]); break;
		case UINT64VAL: dbr.UInt64Val = atoi(Row[i]); break;
		case FLOATVAL:  dbr.FloatVal  = (float)atof(Row[i]); break; //Column type is float so there is no data loss
		case DOUBLEVAL: dbr.DoubleVal = atof(Row[i]); break;
		case RAWPTRVAL: dbr.RawPtrVal = NULL; break; //FIXME
		case BYTESVAL:  dbr.BytesVal  = NULL; break; //FIXME
		case ASCIISTTVAL: 
			{
				if((dbr.AsciiStrVal = (char*)malloc(strlen(Row[i]) + 1)) == NULL)
				{
					this->NotifyError("Could not malloc for result value");
					return false; //FIXME: free already alloc space
				}
				strcpy(dbr.AsciiStrVal, Row[i]);
			} break;
		case UNICSTRVAL: dbr.UnicStrVal = NULL; break; //FIXME
		//case HASHVAL: dbr.HashVal = 0; break; //FIXME
		default:
			return false;
		}
		VectPtr.Push(dbr);
	}
	return true;
}

bool DB_MySQL::FetchNextRow (DbRecordVect &VectPtr)
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

bool DB_MySQL::FetchRowNr (DbRecordVect &VectPtr, UInt32 RowNr)
{
	char Query[1024] = "";
	MYSQL_RES *Res;
	MYSQL_ROW Row;
	bool ret = true;
	char *tmp;

	tmp = strstr(this->DBResBuff.QueryStr, "from");
	if(tmp == NULL)
	{
		this->NotifyError("Invalid query");
		return false;
	}

	sprintf(Query, "select * %s limit %d,1", tmp, RowNr-1);
	if(mysql_query(this->conn, Query)) 
	{
		free(Query);
		this->NotifyError();
		return false;
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

bool DB_MySQL::FreeRow(DbRecordVect &Vect)
{
	for(UInt32 i = 0; i < Vect.Len(); i++)
	{
		switch(Vect[i].Type)
		{
		case BYTESVAL:
		case ASCIISTTVAL:
		case UNICSTRVAL:
		case RAWPTRVAL:
			{
				if(Vect[i].RawPtrVal != NULL)
					free(Vect[i].RawPtrVal);
			}
		}
	}
	return Vect.DeleteAll();
}

bool DB_MySQL::InsertRow (char* Table, DbRecordVect &Vect)
{
	return this->InsertRow(Table, "", Vect);
}

bool DB_MySQL::InsertRow (char* Table, char* Fields, DbRecordVect &Vect)
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
	sprintf(Query, "insert into %s(%s) values(", Table, Fields);
	
	char to_add[3] = ", ";

	for(UInt32 i = 0; i < Vect.Len(); i++)
	{
		if(i == Vect.Len()-1)
			to_add[0] = ')';
		switch(Vect[i].Type)
		{
		case NULLVAL: sprintf(Query, "%sNULL%s", Query, to_add); break;
		case UINT8VAL:  sprintf(Query, "%s'%d'%s", Query, Vect[i].UInt8Val, to_add); break;
		case UINT16VAL: sprintf(Query, "%s'%d'%s", Query, Vect[i].UInt16Val, to_add); break;
		case UINT32VAL: sprintf(Query, "%s'%d'%s", Query, Vect[i].UInt32Val, to_add); break;
		case UINT64VAL: sprintf(Query, "%s'%d'%s", Query, Vect[i].UInt64Val, to_add); break;
		case FLOATVAL:  sprintf(Query, "%s'%f'%s", Query, Vect[i].FloatVal, to_add); break;
		case DOUBLEVAL: sprintf(Query, "%s'%f'%s", Query, Vect[i].DoubleVal, to_add); break;
		case ASCIISTTVAL: sprintf(Query, "%s'%s'%s", Query, Vect[i].AsciiStrVal, to_add); break;
		}
	}

	if(mysql_query(this->conn, Query))
	{
		this->NotifyError();
		return false;
	}

	return true;
}

bool DB_MySQL::Update (char* SqlStatement, DbRecordVect &WhereVals, DbRecordVect &UpdateVals)
{
	return false;
}

void DB_MySQL::NotifyError(char* Msg)
{
	if(Msg == "")
		this->notifier->Notify((char*)mysql_error(this->conn));
	else
		this->notifier->Notify(Msg);
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
			strcpy(tmp, Query);
			memset(Query, 0, slen);
			strncpy(Query, tmp, i);
			strcpy(&Query[i], &tmp[i+j]);
			return true;
		}
continue_iter:
		1==1;
	}
	return false;
}