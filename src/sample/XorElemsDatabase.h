#ifndef __XORELEMSDATABASE_H
#define __XORELEMSDATABASE_H

#include "gml.h"

class XorElemsDatabase: public IDatabase
{
private:
	UInt32 currentRow;

public:
	XorElemsDatabase();
	~XorElemsDatabase();

	 virtual UInt32 SqlSelect( char* What="*", char* Where="", char* From="" );	 
	 virtual UInt32 Select (char* Statement="*");

	 virtual bool Update( char* SqlStatement, DbRecordVect &WhereVals, DbRecordVect &UpdateVals );

	 virtual bool FreeRow( DbRecordVect &Vect );

	 virtual bool Disconnect();

	 virtual bool Init(INotifier &notifier, char* Database="", char* Server="", char* Username="", char* Password="", UInt32 Port=0 );

	 virtual bool Connect();

	 virtual bool FetchNextRow( DbRecordVect &VectPtr );

	 virtual bool FetchRowNr( DbRecordVect &VectPtr, UInt32 RowNr );

	 virtual bool InsertRow( char* Table, char* Fields, DbRecordVect &Vect );
	 virtual bool InsertRow (char* Table, DbRecordVect &Vect);

};

#endif