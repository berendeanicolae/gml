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
	GML::Utils::CacheFile	file;
	GML::Utils::GString		fileName;
	GML::Utils::GString		line,token;
	UInt32					CacheSize;
	UInt64					DataStart,CurentPos;

public:
	SimpleTextFileDB();

	bool				OnInit();
	bool				Close();
	bool				BeginIteration();
	bool				ReadNextRecord(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr);
};
 

#endif
