// gml.cpp : Defines the entry point for the console application.
//

#include "gml.h"

#include <stdio.h>
#include <string.h>

Int32 main(Int32 argc, Int8* argv[])
{
	char temp[128];

	ConsoleNotifier cn;

	XorElemsDatabase db;
	DbRecordVect vect;

	db.SqlSelect();
	
	db.FetchNextRow(vect);
	sprintf(temp, "%d %d\n", (DbRecord*)vect.GetPtrToObject(0)->UInt32Val, (DbRecord*)vect.GetPtrToObject(1)->UInt32Val);
	cn.Notify(temp);

	vect.DeleteAll();
	db.FetchNextRow(vect);
	sprintf(temp, "%d %d\n", (DbRecord*)vect.GetPtrToObject(0)->UInt32Val, (DbRecord*)vect.GetPtrToObject(1)->UInt32Val);
	cn.Notify(temp);
	

	return 0;
}

