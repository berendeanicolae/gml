#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "gmllib.h"
#include "sqlite.h"
#include "sqlite3.h"

#define xtod(c) ((c>='0' && c<='9') ? c-'0' : ((c>='A' && c<='F') ? c-'A'+10 : ((c>='a' && c<='f') ? c-'a'+10 : 0)))
//int HexToDec(char* hex, int l){return (*hex==0 ? l : HexToDec(hex + 1, l * 15 + xtod(*hex)));}

int HexToDec (char ch) {
	if (ch >= '0' && ch <= '9') return ch - 0x30;	
	if (ch >= 'a' && ch <= 'f') return ch - 0x57;
	if (ch >= 'A' && ch <= 'F') return ch - 0x37;

	return -1;
}

bool TextToInt(char* str, struct GML::DB::RecordHash& hash)
{

	for(UInt32 i = 0; i < 32; i = i+2)
	{			
		int v1 = HexToDec(str[i]);
		int v2 = HexToDec(str[i+1]);
		if (v1==-1 || v2==-1) return false;
		int val = v1*16+v2;
		hash.Hash.bValue[i/2] = val;
	}
	return true;
}

SqliteDatabase::SqliteDatabase()
{
	database_name = NULL;
	tail = NULL;
	database = NULL;
	res = NULL;
	ObjectName = "SqliteDatabase";

	LinkPropertyToString("FileName",FileName,"","File that contains the sqllite database");
}
SqliteDatabase::~SqliteDatabase()
{
    if(this->database_name != NULL)
    {
        free(this->database_name);
    }
    if(this->tail != NULL)
    {
        free(this->tail);
    }
    this->Disconnect();
}
bool SqliteDatabase::OnInit()
{
	char* _text = FileName.GetText();
	if(NULL == _text){
		notifier->Error("FileName attribute does not exists!");
		return false;
	}
	this->database_name = (char*)malloc(sizeof(char) * strlen(_text));
	if(this->database_name == NULL)
	{
		notifier->Error("Error, could not allocate memory for private buffer!");
		return false;
	}	
	strcpy((char*)this->database_name, _text);
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
		notifier->Error("Error: could not connect to database.");
		return false;
	}
	return error != SQLITE_OK ? false : true;
}

UInt32 SqliteDatabase::Select(char* Statement)
{
	
	UInt32 error = 0;
	sqlite3_stmt *new_res;
	error = sqlite3_prepare_v2(this->database, Statement, strlen(Statement), &new_res, (const char**)&this->tail);
	if (error != SQLITE_OK)
	{
		notifier->Error("Failed to get data from database!");
		return 0;
	}
	UInt32 counter = 0;
	while((sqlite3_step(new_res) == SQLITE_ROW))
	{
		counter++;
	}
	sqlite3_finalize(new_res);	
	error = sqlite3_prepare_v2(this->database, Statement, strlen(Statement), &this->res, (const char**)&this->tail);
	if (error != SQLITE_OK)
	{
		notifier->Error("Failed to get data from database!");
	}
	return counter;
}

UInt32 SqliteDatabase::SqlSelect(char* What, char* Where, char* From)
{

	char* statement = (char*)malloc((30 + strlen(What) + strlen(Where) + strlen(From)) * sizeof(char));
	if (statement == NULL)
	{
		notifier->Error("Memory allocaton failed!");
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

bool SqliteDatabase::FetchNextRow(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr)
{
	UInt32 result = 0;
	UInt32 column_count = 0;
	result = sqlite3_step(this->res);
	if (result == SQLITE_ROW)
	{
		column_count = sqlite3_column_count(this->res);
		for(UInt32 i=0; i < column_count; i++)
		{
			GML::DB::DBRecord rec;
			UInt32 current_type = 0;
			current_type = sqlite3_column_type(this->res, i);
			rec.Name = (char*)sqlite3_column_name(this->res, i);
			if (strcmp (rec.Name, "RecId") == 0)
			{
				continue;
			}			
			switch(current_type)
			{				
				case SQLITE_TEXT:
					if (strcmp (rec.Name, "Hash") != 0)
					{
						current_type = GML::DB::TYPES::ASCII;
						rec.Value.AsciiStrVal = (char*)sqlite3_column_text(this->res, i);
					}
					else
					{					
						current_type = GML::DB::TYPES::HASH;		
						struct GML::DB::RecordHash nhash = {};
						if (TextToInt((char*)sqlite3_column_text(this->res, i), nhash) == false) {
							notifier->Error ("error convering ascii string to Hash value");
							return false;
						}
						rec.Value.Hash = nhash;
					}
                    break;
				case SQLITE_FLOAT:
                    current_type = GML::DB::TYPES::DOUBLE;					
                    rec.Value.DoubleVal = (double)sqlite3_column_double(this->res, i); // To be replaced with floatval
                    break;
				case SQLITE_NULL:
                    current_type = GML::DB::NULLVAL;
                    break;
				case SQLITE_BLOB:					
                    current_type = GML::DB::BYTESVAL;
					rec.BytesVal = (UInt8*)sqlite3_column_blob(this->res, i);     
					rec.Size = sqlite3_column_bytes(this->res, 0);
                    break;
				case SQLITE_INTEGER:
                    current_type = GML::DB::TYPES::INT32;
                    rec.Value.UInt32Val = (UInt32)sqlite3_column_int(this->res, i);
                    break;
				default:					
					notifier->Error("Unknown type of column.");
			}			
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
bool SqliteDatabase::FetchRowNr(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr, UInt32 RowNr)
{
	bool result = this->FetchNextRow(VectPtr);
	if (false == result)
	{
		notifier->Error("No row fetched from previous operation.");
		return false;
	}
	if (RowNr > VectPtr.GetCount())
	{
		notifier->Error("Index Error: out of range");
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
bool SqliteDatabase::FreeRow( GML::Utils::GTFVector<GML::DB::DBRecord> &Vect )
{
	UInt32 counter = Vect.GetCount();
	for(UInt32 i = 0; i < counter; i ++)
	{
        UInt32 type = Vect[i].Type;
        switch(type)
        {
			case GML::DB::RAWPTRVAL:
                free(Vect[i].RawPtrVal);
                break;
            case GML::DB::BYTESVAL:
                free(Vect[i].BytesVal);
                break;
            case GML::DB::UNICSTRVAL:
                free(Vect[i].UnicStrVal);
                break;
            default:
                continue;
        }
	}
	Vect.DeleteAll();
	return counter > 0 ? true :false;
}
bool SqliteDatabase::_InsertRow(char* Table, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect, char* Fields)
{
	if(!this->database)
	{

		notifier->Error("The database was closed or destroyed.");
		return false;
    }
	UInt32 error = 0;
	UInt32 count = Vect.GetCount();
	if (0 == count)
	{
	    notifier->Error("No values in vector!");
	    return false;
	}
	char statement[1024];
	sprintf(statement, Fields != "" ? "INSERT INTO %s(%s) VALUES(" : "INSERT INTO %s%s VALUES(", Table, Fields);
	for(UInt32 i = 0; i < Vect.GetCount(); i++)
	{
	    switch(Vect[i].Type)
	    {		
	        case GML::DB::TYPES::UINT8: sprintf(statement, "%s %d, ", statement, Vect[i].Value.UInt8Val); break;
	        case GML::DB::TYPES::UINT16: sprintf(statement, "%s %d, ", statement, Vect[i].Value.UInt16Val); break;
	        case GML::DB::TYPES::UINT32: sprintf(statement, "%s %d, ", statement, Vect[i].Value.UInt32Val); break;
	        case GML::DB::TYPES::UINT64: sprintf(statement, "%s %d, ", statement, Vect[i].Value.UInt64Val); break;
	        case GML::DB::TYPES::ASCII: sprintf(statement, "%s '%s', ", statement, Vect[i].Value.AsciiStrVal); break;
	        case GML::DB::TYPES::UNICODE: sprintf(statement, "%s '%s', ", statement, Vect[i].Value.UnicStrVal); break;
	        case GML::DB::NULLVAL: sprintf(statement, "%sNULL, ", statement); break;
			case GML::DB::BYTESVAL: 
				{
					char* _bytes_val_statement = (char*)malloc(sizeof(char) * (Vect[i].Size * 4) + 1);					
					if(_bytes_val_statement == NULL)
					{
						notifier->Error("Could not allocate memory for blob object!");
						return false;
					}
					// init private buffer;
					sprintf(_bytes_val_statement, "");
					for(UInt32 x = 0; x < Vect[i].Size; x++)
					{
						sprintf(_bytes_val_statement, "%s0x%02x", _bytes_val_statement, Vect[i].BytesVal[x]);
					}
					sprintf(statement, "%s '%s', ", statement, _bytes_val_statement); 
					if(_bytes_val_statement != NULL)
					{
						free(_bytes_val_statement);
					}
					break;
				}

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

bool SqliteDatabase::InsertRow(char* Table, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect)
{
	return this->_InsertRow(Table, Vect, "");
}

bool SqliteDatabase::InsertRow(char* Table, char* Fields, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect)
{
	return this->_InsertRow(Table, Vect, Fields);
}


bool SqliteDatabase::Update(char* SqlStatement, GML::Utils::GTFVector<GML::DB::DBRecord> &WhereVals, GML::Utils::GTFVector<GML::DB::DBRecord> &UpdateVals )
{
	if(!this->database)
	{
		notifier->Error("The database was closed or destroyed.");
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
	        case GML::DB::TYPES::UINT8: sprintf(buffer_w, "%s %s %s = %d ", buffer_w, i == 0 ? "" : "and", WhereVals[i].Name, WhereVals[i].Value.UInt8Val); break;
	        case GML::DB::TYPES::UINT16: sprintf(buffer_w, "%s %s %s = %d ", buffer_w, i == 0 ? "" : "and", WhereVals[i].Name, WhereVals[i].Value.UInt16Val); break;
	        case GML::DB::TYPES::UINT32: sprintf(buffer_w, "%s %s %s = %d ", buffer_w, i == 0 ? "" : "and", WhereVals[i].Name, WhereVals[i].Value.UInt32Val); break;
	        case GML::DB::TYPES::UINT64: sprintf(buffer_w, "%s %s %s = %d ", buffer_w, i == 0 ? "" : "and", WhereVals[i].Name, WhereVals[i].Value.UInt64Val); break;
	        case GML::DB::TYPES::ASCII: sprintf(buffer_w, "%s %s %s = '%s' ", buffer_w, i == 0 ? "" : "and", WhereVals[i].Name, WhereVals[i].Value.AsciiStrVal); break;
	        case GML::DB::TYPES::UNICODE: sprintf(buffer_w, "%s %s %s = '%s' ", buffer_w, i == 0 ? "" : "and", WhereVals[i].Name, WhereVals[i].Value.UnicStrVal); break;
	        case GML::DB::NULLVAL: sprintf(buffer_w, "%s %s %s = NULL ", buffer_w, i == 0 ? "" : "and", WhereVals[i].Name); break;
			case GML::DB::BYTESVAL: 
				{
					// abc 0xXX0xXX0xXX = > size * 4
					char* _bytes_val_statement = (char*)malloc(sizeof(char) * (WhereVals[i].Size * 4) + 1);
					if(_bytes_val_statement == NULL)
					{
						notifier->Error("Could not allocate memory for blob object!");
						return false;
					}
					// init private buffer
					sprintf(_bytes_val_statement, "");
					for(UInt32 x=0; x < WhereVals[i].Size; x++)
					{
						sprintf(_bytes_val_statement, "%s0x%02x", _bytes_val_statement, WhereVals[i].BytesVal[x]);
					}
					sprintf(buffer_w, "%s %s %s = '%s' ", buffer_w, i == 0 ? "" : ",", WhereVals[i].Name, _bytes_val_statement); 
					free(_bytes_val_statement);
					break;
				}
	        default: continue;

        }
	}	
	for(UInt32 i = 0; i < length_u ; i++)
	{
        switch(UpdateVals[i].Type)
        {
	        case GML::DB::TYPES::UINT8: sprintf(buffer_u, "%s %s %s = %d ", buffer_u, i == 0 ? "" : ",", UpdateVals[i].Name, UpdateVals[i].Value.UInt8Val); break;
	        case GML::DB::TYPES::UINT16: sprintf(buffer_u, "%s %s %s = %d ", buffer_u, i == 0 ? "" : ",", UpdateVals[i].Name, UpdateVals[i].Value.UInt16Val); break;
	        case GML::DB::TYPES::UINT32: sprintf(buffer_u, "%s %s %s = %d ", buffer_u, i == 0 ? "" : ",", UpdateVals[i].Name, UpdateVals[i].Value.UInt32Val); break;
	        case GML::DB::TYPES::UINT64: sprintf(buffer_u, "%s %s %s = %lld ", buffer_u, i == 0 ? "" : ",", UpdateVals[i].Name, UpdateVals[i].Value.UInt64Val); break;
	        case GML::DB::TYPES::ASCII: sprintf(buffer_u, "%s %s %s = '%s' ", buffer_u, i == 0 ? "" : ",", UpdateVals[i].Name, UpdateVals[i].Value.Ascii); break;
	        case GML::DB::TYPES::UNICODE: sprintf(buffer_u, "%s %s %s = '%s' ", buffer_u, i == 0 ? "" : ",", UpdateVals[i].Name, UpdateVals[i].Value.Unicode); break;
	        case GML::DB::TYPES::NULLVAL: sprintf(buffer_u, "%s %s %s = NULL ", buffer_u, i == 0 ? "" : ",", UpdateVals[i].Name); break;
			case GML::DB::TYPES::BYTESVAL: 
				{								
					char* _bytes_val_statement = (char*)malloc(sizeof(char) * (UpdateVals[i].Size * 4) + 1);
					if(_bytes_val_statement == NULL)
					{
						notifier->Error("Could not allocate memory for blob object!");
						return false;
					}
					// init private buffer
					sprintf(_bytes_val_statement, "");

					for(UInt32 x=0; x < UpdateVals[i].Size; x++)
					{
						sprintf(_bytes_val_statement, "%s0x%02x", _bytes_val_statement, UpdateVals[i].BytesVal[x]);
					}
					sprintf(buffer_u, "%s %s %s = '%s' ", buffer_u, i == 0 ? "" : ",", UpdateVals[i].Name, _bytes_val_statement); 
					free(_bytes_val_statement);
					break;
				}

	        default: continue;

        }
	}
	UInt32 __k = (strlen(SqlStatement) + strlen(buffer_u) + strlen(buffer_w)) * sizeof(char) + 16;
	char* update_statement = (char*)malloc(__k);
	if (NULL == update_statement)
	{
		notifier->Error("Failed to allocate memory for SqlStatement");
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

bool SqliteDatabase::GetColumnInformations( char* TableName, GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr )
{
	char* statement = (char*)malloc(sizeof(char) * 15 + strlen(TableName));
	if(NULL == statement)
	{
		notifier->Error("Failed to allocate memory for SqlStatement");
		return false;
	}
	sprintf(statement, "");
	sprintf(statement, "select * from %s", TableName);		
	UInt32 counter = this->Select(statement);	
	UInt32 column_count = 0;
	UInt32 result = sqlite3_step(this->res);
	if(result == SQLITE_ROW)
	{
		column_count = sqlite3_column_count(this->res);
		for(UInt32 i=0; i < column_count; i++)
		{
			GML::DB::DBRecord rec;
			UInt32 current_type = 0;
			current_type = sqlite3_column_type(this->res, i);
			rec.Name = (char*)sqlite3_column_name(this->res, i);				
			switch(current_type)
			{							
				case SQLITE3_TEXT:
					if (strcmp (rec.Name, "Hash") != 0)
					{
						current_type = GML::DB::TYPES::ASCII;					
					}
					else
					{					
						current_type = GML::DB::TYPES::HASH;		
					}
					break;
				case SQLITE_FLOAT:
					current_type = GML::DB::TYPES::DOUBLE;									
					break;
				case SQLITE_NULL:
					current_type = GML::DB::NULLVAL;
					break;
				case SQLITE_BLOB:					
					current_type = GML::DB::BYTESVAL;
					break;
				case SQLITE_INTEGER:
					current_type = GML::DB::TYPES::INT32;				
					break;
				default:					
					notifier->Error("Unknown type of column.");
			}			
			rec.Type = current_type;
			VectPtr.Push(rec);
		}
	}
	else
	{
		free(statement);
		return false;
	}
	
	free(statement);
	return true;
}


