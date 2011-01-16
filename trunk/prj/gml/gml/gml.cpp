// gml.cpp : Defines the entry point for the console application.
//

#include "gml.h"

#include <stdio.h>
#include <string.h>

Int32 main(Int32 argc, Int8* argv[])
{	

	ConsoleNotifier		cn;
	XorElemsDatabase	db;
	XorMlDatabase xorml;

	db.Init(cn);
	xorml.Init(cn, db);

	MLRecord mlr1, mlr0;
	
	xorml.CreateMlRecord(mlr0);
	xorml.GetRecord(mlr0, 0);

	xorml.CreateMlRecord(mlr1);
	xorml.GetRecord(mlr1, 1);

	char temp[100];

	sprintf(temp, "record %d -> %.02f %.02f Label: %.02f\n", 0, mlr0.Features[0], mlr0.Features[1],mlr0.Label);
	cn.Notify(temp);
	xorml.FreeMLRecord(mlr0);

	sprintf(temp, "record %d -> %.02f %.02f Label: %.02f\n", 1, mlr1.Features[0], mlr1.Features[1], mlr1.Label);
	cn.Notify(temp);
	xorml.FreeMLRecord(mlr1);


	return 0;
}

