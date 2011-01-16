
#include "XorElemsDatabase.h"

UInt32 XorElemsDatabase::SqlSelect( char* What, char* Where, char* From)
{
	currentRow = 0;
	return 2;
}

UInt32 XorElemsDatabase::Select( char* SqlStatement/*="*"*/ )
{
	currentRow = 0;
	return 2;
}

bool XorElemsDatabase::Update(char* SqlStatement, DbRecordVect &WhereVals, DbRecordVect &UpdateVals)
{
	return false;
}

bool XorElemsDatabase::FreeRow(DbRecordVect &Vect)
{
	return true;
}

bool XorElemsDatabase::Disconnect()
{
	return true;
}

bool XorElemsDatabase::Init(INotifier &notifier, char* Database, char* Server, char* Username, char* Password, UInt32 Port)
{
	this->notifier = &notifier;
	return true;
}

bool XorElemsDatabase::Connect()
{
	return true;
}

bool XorElemsDatabase::FetchNextRow( DbRecordVect &VectPtr )
{
	if (currentRow >1)
	{
		VectPtr.DeleteAll();
		return false;	
	}
	
	if (currentRow==0)
	{
		currentRow++;
		return FetchRowNr(VectPtr, 0);
	}
	else
	{
		currentRow++;
		return FetchRowNr(VectPtr, 1);
	}
	
	return false;
}

bool XorElemsDatabase::FetchRowNr( DbRecordVect &vect, UInt32 RowNr )
{	
	DbRecord	 rec1, rec2;

	if (RowNr==0) 
	{		
		rec1.UInt32Val = 0;
		rec1.Name = "COL0";
		rec1.Type = DbDataType::UINT32VAL;

		rec2.UInt32Val = 1;
		rec2.Name = "COL1";
		rec2.Type = DbDataType::UINT32VAL;

		vect.Push(rec1);
		vect.Push(rec2);		

		notifier->Notify("fetched record number 0\n");

		return true;
	} else 
		if (RowNr == 1) 
		{		
			rec1.UInt32Val = 1;
			rec1.Name = "COL0";
			rec1.Type = DbDataType::UINT32VAL;

			rec2.UInt32Val = 0;
			rec2.Name = "COL1";
			rec2.Type = DbDataType::UINT32VAL;

			vect.Push(rec1);
			vect.Push(rec2);

			notifier->Notify("fetched record number 1\n");

			return true;
		}

	vect.DeleteAll();
	return false;
}

bool XorElemsDatabase::InsertRow( char* Table, char* Fields, DbRecordVect &Vect )
{
	return false;
}

bool XorElemsDatabase::InsertRow( char* Table, DbRecordVect &Vect )
{
	return false;
}

XorElemsDatabase::XorElemsDatabase()
{
	currentRow = 0;
}

XorElemsDatabase::~XorElemsDatabase()
{
	
}
