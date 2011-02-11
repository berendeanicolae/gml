#ifndef __SIMPLE_TEXT_FILE_DB__
#define __SIMPLE_TEXT_FILE_DB__

#include "gmllib.h"

/**
	Fisierele au un format foarte simplu:
	* primele 2 linii trebuie sa fie obligatoriu urmatoarele
		Records = <numar de recorduri>
		Features = <numar de feature-uri>
	* urmeaza apoi toate inregistrarile cu urmatorul format:
		label: lista featureruri setata (0 based index)
		label poate fi 1 sau -1 (pentru a indica una din cele doua clase)
	EXEMPLU:
		Records = 3
		Features = 4
		1: 2,3
		-1: 0,2,1
		1: 1

**/

class SimpleTextFileDB: public GML::DB::IDataBase
{
	GML::Utils::File		file;
	GML::Utils::GString		fileName,tempStr,featureIndex;
	UInt32					nrRecords,nrFeatures;
	UInt32					cIndex;
	GML::Utils::GString*	FeatNames;

public:
	bool				OnInit();
	bool				Connect ();
	bool				Disconnect ();
	UInt32				Select (char* Statement="*");
	UInt32				SqlSelect (char* What="*", char* Where="", char* From="");
	bool				FetchNextRow (GML::Utils::GTVector<GML::DB::DBRecord> &VectPtr);
	bool				FetchRowNr (GML::Utils::GTVector<GML::DB::DBRecord> &VectPtr, UInt32 RowNr);
	bool				FreeRow(GML::Utils::GTVector<GML::DB::DBRecord> &Vect);
	bool				InsertRow (char* Table, GML::Utils::GTVector<GML::DB::DBRecord> &Vect);
	bool				InsertRow (char* Table, char* Fields, GML::Utils::GTVector<GML::DB::DBRecord> &Vect);
	bool				Update (char* SqlStatement, GML::Utils::GTVector<GML::DB::DBRecord> &WhereVals, GML::Utils::GTVector<GML::DB::DBRecord> &UpdateVals);

};
 

#endif
