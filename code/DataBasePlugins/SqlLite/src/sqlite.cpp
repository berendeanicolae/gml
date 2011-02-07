#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "gmllib.h"
#include "sqlite.h"
#include "sqlite3.h"
using namespace GML::DB;

SqliteDatabase::SqliteDatabase()
{
}
GML::Utils::AttributeList::AttributeList()
{
}
GML::Utils::AttributeList::~AttributeList()
{
}
SqliteDatabase::~SqliteDatabase()
{
    if(this->database_name != NULL)
    {
        free(&this->database_name);
    }
    if(this->tail != NULL)
    {
        free(&this->tail);
    }
    this->Disconnect();
}
bool SqliteDatabase::OnInit()
{
	// we don't need other infos
	return true;
}
bool SqliteDatabase::Init(GML::Utils::INotify &notifier, char* connectionString)
{	
	this->notifier = &notifier;
	this->database_name = connectionString;
	return true;
}
bool SqliteDatabase::Finalize()
{

	// prevent leakage by destroying our prepared statement
	if(NULL != this->res)
	{
		UInt32 error = sqlite3_finalize(this->res);
		return error == SQLITE_OK ? true : false;
	}
	return true;
}

bool SqliteDatabase::Disconnect()
{
	UInt32 error = -1;
	if(NULL != this->database)
	{
		error = sqlite3_close(this->database);		
	}
	return error != SQLITE_OK ? false : true;
}

bool SqliteDatabase::Connect()
{
	UInt32 error = 0;
	error = sqlite3_open(database_name, &database);
	if (error != SQLITE_OK)
	{
		char* err_mes = "Error: could not connect to database.";
		UInt32 len = strlen(err_mes);
		notifier->Notify(0, err_mes, 0);
		return false;
	}
	return error != SQLITE_OK ? false : true;
}

UInt32 SqliteDatabase::Select(char* Statement)
{
	UInt32 error = 0;
	error = sqlite3_prepare_v2(this->database, Statement, 10000, &this->res, &this->tail);
	if (error != SQLITE_OK)
	{
		char* err_mes = "Failed to get data from database!";
		UInt32 len = strlen(err_mes);
		notifier->Notify(0, err_mes, len);
	}
	return error;
}

UInt32 SqliteDatabase::SqlSelect(char* What, char* Where, char* From)
{

	char* statement = (char*)malloc((30 + strlen(What) + strlen(Where) + strlen(From)) * sizeof(char));
	if (statement == NULL)
	{
		char* err_mes = "Memory allocaton failed!";
		UInt32 len = strlen(err_mes);
		notifier->Notify(0, err_mes, len);
		return 0;
	}
	sprintf(statement, "SELECT %s FROM %s WHERE %s", What, From, Where);
	UInt32 error = this->Select(statement);
	if(statement != NULL)
	{
		free(statement);
	}
	return 1; // convenience return. I can't find out the nmber of results without iterating through them. That's how sqlite does the work
}

bool SqliteDatabase::FetchNextRow(GML::Utils::GTVector<GML::DB::DBRecord> &VectPtr)
{
	UInt32 result = 0;
	UInt32 column_count = 0;
	this->FreeRow(VectPtr);	
	result = sqlite3_step(this->res);
	if (result == SQLITE_ROW)
	{
		column_count = sqlite3_column_count(this->res);
		for(UInt32 i=0; i < column_count; i++)
		{
			GML::DB::DBRecord rec;
			UInt32 current_type = 0;
			current_type = sqlite3_column_type(this->res, i);
			switch(current_type)
			{
				
				case SQLITE_TEXT:
                    current_type = ASCIISTTVAL;
                    rec.AsciiStrVal = (char*)sqlite3_column_text(this->res, i);
                    break;
				case SQLITE_FLOAT:
                    current_type = UINT32VAL;					
                    rec.FloatVal = (float)sqlite3_column_double(this->res, i); // To be replaced with floatval
                    break;
				case SQLITE_NULL:
                    current_type = NULLVAL;
                    break;
				case SQLITE_BLOB:
                    current_type = UNICSTRVAL;
                    rec.UnicStrVal = (wchar_t*)sqlite3_column_blob(this->res, i);
                    break;
				case SQLITE_INTEGER:
                    current_type = UINT32VAL;
                    rec.UInt32Val = (int)sqlite3_column_int(this->res, i);
                    break;
				default:
					char* err_mes = "Unknown type of column.";
					UInt32 len = strlen(err_mes);
                    notifier->Notify(0, err_mes, len);
			}
			rec.Name = (char*)sqlite3_column_name(this->res, i);
			rec.Type = current_type;
			VectPtr.Push(rec);
		}
		return true;
	}
	else
	{
		return false;
	}
}
bool SqliteDatabase::FetchRowNr(GML::Utils::GTVector<GML::DB::DBRecord> &VectPtr, UInt32 RowNr)
{
	bool result = this->FetchNextRow(VectPtr);
	if (false == result)
	{
		char* err_mes = "No row fetched from previous operation.";
		UInt32 len = strlen(err_mes);
		notifier->Notify(0, err_mes, len);
		return false;
	}
	if (RowNr > VectPtr.GetCount())
	{
		char* err_mes = "Index Error: out of range";
		UInt32 len = strlen(err_mes);
		notifier->Notify(0, err_mes, len);
		return false;
	}
	else
	{
		UInt32 counter = 0;
		while((this->FetchNextRow(VectPtr) == true))
		{
			if(counter == RowNr)
			{
				break;
            }
			counter++;
        }
		return true;
    }
}
bool SqliteDatabase::FreeRow( GML::Utils::GTVector<GML::DB::DBRecord> &Vect )
{
	UInt32 counter = Vect.GetCount();
	for(UInt32 i = 0; i < counter; i ++)
	{
        UInt32 type = Vect[i].Type;
        switch(type)
        {
			case RAWPTRVAL:
                free(Vect[i].RawPtrVal);
                break;
            case BYTESVAL:
                free(Vect[i].BytesVal);
                break;
            case UNICSTRVAL:
                free(Vect[i].UnicStrVal);
                break;
            default:
                continue;
        }
	}
	Vect.DeleteAll();
	return counter > 0 ? true :false;
}
bool SqliteDatabase::_InsertRow(char* Table, GML::Utils::GTVector<GML::DB::DBRecord> &Vect, char* Fields)
{
	if(!this->database)
	{
		char* err_mes = "The database was closed or destroyed.";
		UInt32 len = strlen(err_mes);
		notifier->Notify(0, err_mes, len);
		return false;
    }
	UInt32 error = 0;
	UInt32 count = Vect.GetCount();
	if (0 == count)
	{
		char* err_mes = "No values in vector!\n";
		UInt32 len = strlen(err_mes);
	    notifier->Notify(0, err_mes, len);
	    return false;
	}
	char statement[1024];
	sprintf(statement, Fields != "" ? "INSERT INTO %s(%s) VALUES(" : "INSERT INTO %s%s VALUES(", Table, Fields);
	for(UInt32 i = 0; i < Vect.GetCount(); i++)
	{
	    switch(Vect[i].Type)
	    {			
	        case UINT8VAL: sprintf(statement, "%s %d, ", statement, Vect[i].UInt8Val); break;
	        case UINT16VAL: sprintf(statement, "%s %d, ", statement, Vect[i].UInt16Val); break;
	        case UINT32VAL: sprintf(statement, "%s %d, ", statement, Vect[i].UInt32Val); break;
	        case UINT64VAL: sprintf(statement, "%s %d, ", statement, Vect[i].UInt64Val); break;
	        case ASCIISTTVAL: sprintf(statement, "%s '%s', ", statement, Vect[i].AsciiStrVal); break;
	        case UNICSTRVAL: sprintf(statement, "%s '%s', ", statement, Vect[i].UnicStrVal); break;
	        case NULLVAL: sprintf(statement, "%sNULL, ", statement); break;
	        default: continue;
	    }
    }
    //strip last trailing comma and space
    statement[strlen(statement)] = '\0';
    statement[strlen(statement) - 2] = '\0';
    sprintf(statement, "%s);", statement);
	error = sqlite3_exec(this->database, statement, 0, 0, 0);
	return error != SQLITE_OK ? false :true;
	}

bool SqliteDatabase::InsertRow(char* Table, GML::Utils::GTVector<GML::DB::DBRecord> &Vect)
{
	return this->_InsertRow(Table, Vect, "");
}

bool SqliteDatabase::InsertRow(char* Table, char* Fields, GML::Utils::GTVector<GML::DB::DBRecord> &Vect)
{
	return this->_InsertRow(Table, Vect, Fields);
}


bool SqliteDatabase::Update(char* SqlStatement, GML::Utils::GTVector<GML::DB::DBRecord> &WhereVals, GML::Utils::GTVector<GML::DB::DBRecord> &UpdateVals )
{
	if(!this->database)
	{
		char* err_mes = "The database was closed or destroyed.";
		UInt32 len = strlen(err_mes);
		notifier->Notify(0, err_mes, len);
		return false;
    }

	UInt32 length_w = WhereVals.GetCount();
	UInt32 length_u = UpdateVals.GetCount();
	char buffer_w[1024] = "";
	char buffer_u[1024] = "";	
	for(UInt32 i = 0; i < length_w ; i++)
	{
        switch(WhereVals[i].Type)
        {
	        case UINT8VAL: sprintf(buffer_w, "%s %s %s = %d ", buffer_w, i == 0 ? "" : "and", WhereVals[i].Name, WhereVals[i].UInt8Val); break;
	        case UINT16VAL: sprintf(buffer_w, "%s %s %s = %d ", buffer_w, i == 0 ? "" : "and", WhereVals[i].Name, WhereVals[i].UInt16Val); break;
	        case UINT32VAL: sprintf(buffer_w, "%s %s %s = %d ", buffer_w, i == 0 ? "" : "and", WhereVals[i].Name, WhereVals[i].UInt32Val); break;
	        case UINT64VAL: sprintf(buffer_w, "%s %s %s = %d ", buffer_w, i == 0 ? "" : "and", WhereVals[i].Name, WhereVals[i].UInt64Val); break;
	        case ASCIISTTVAL: sprintf(buffer_w, "%s %s %s = '%s' ", buffer_w, i == 0 ? "" : "and", WhereVals[i].Name, WhereVals[i].AsciiStrVal); break;
	        case UNICSTRVAL: sprintf(buffer_w, "%s %s %s = '%s' ", buffer_w, i == 0 ? "" : "and", WhereVals[i].Name, WhereVals[i].UnicStrVal); break;
	        case NULLVAL: sprintf(buffer_w, "%s %s %s = NULL ", buffer_w, i == 0 ? "" : "and", WhereVals[i].Name); break;
	        default: continue;

        }
	}	
	for(UInt32 i = 0; i < length_u ; i++)
	{
        switch(UpdateVals[i].Type)
        {
	        case UINT8VAL: sprintf(buffer_u, "%s %s %s = %d ", buffer_u, i == 0 ? "" : ",", UpdateVals[i].Name, UpdateVals[i].UInt8Val); break;
	        case UINT16VAL: sprintf(buffer_u, "%s %s %s = %d ", buffer_u, i == 0 ? "" : ",", UpdateVals[i].Name, UpdateVals[i].UInt16Val); break;
	        case UINT32VAL: sprintf(buffer_u, "%s %s %s = %d ", buffer_u, i == 0 ? "" : ",", UpdateVals[i].Name, UpdateVals[i].UInt32Val); break;
	        case UINT64VAL: sprintf(buffer_u, "%s %s %s = %lld ", buffer_u, i == 0 ? "" : ",", UpdateVals[i].Name, UpdateVals[i].UInt64Val); break;
	        case ASCIISTTVAL: sprintf(buffer_u, "%s %s %s = '%s' ", buffer_u, i == 0 ? "" : ",", UpdateVals[i].Name, UpdateVals[i].AsciiStrVal); break;
	        case UNICSTRVAL: sprintf(buffer_u, "%s %s %s = '%s' ", buffer_u, i == 0 ? "" : ",", UpdateVals[i].Name, UpdateVals[i].UnicStrVal); break;
	        case NULLVAL: sprintf(buffer_u, "%s %s %s = NULL ", buffer_u, i == 0 ? "" : ",", UpdateVals[i].Name); break;
	        default: continue;

        }
	}
	int __k = (strlen(SqlStatement) + strlen(buffer_u) + strlen(buffer_w)) * sizeof(char) + 16;
	char* update_statement = (char*)malloc(__k);
	if (NULL == update_statement)
	{
		char* err_mes = "Failed to allocate memory for SqlStatement";
		UInt32 len = strlen(err_mes);
		notifier->Notify(0, err_mes, len);
		return false;
	}	
	sprintf(update_statement, "%s set %s where %s;", SqlStatement, buffer_u, buffer_w);
	UInt32 error = sqlite3_exec(this->database, update_statement, 0, 0, 0);	
	if(update_statement != NULL)
	{ 
		free(update_statement);
	}		
	return error != SQLITE_OK ? false : true;
}

extern "C" __declspec(dllexport) SqliteDatabase&  Create(GML::Utils::INotify* notifier, char* connectionString)
{	
	SqliteDatabase* newobj = new SqliteDatabase();
	newobj->Init(*notifier, connectionString);
	return *newobj;
}

 extern "C" __declspec(dllexport) bool Destroy(SqliteDatabase& db)
{
	return db.Disconnect();
	delete &db;
	return true;
}

