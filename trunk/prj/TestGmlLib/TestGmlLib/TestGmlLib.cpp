// TestGmlLib.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "gmllib.h"



int _tmain(int argc, _TCHAR* argv[])
{
	int x[10]; //= { 1,5,2,6,3,4,8,9,0,11 };
	GML::Utils::AttributeList	a;

	GML::Utils::INotify *notif = GML::Builder::CreateNotifyer("E:\\lucru\\GML\\gml\\lib\\ConsoleNotifyer.ntf");
	if (notif)
		notif->NotifyString(0,"Mesaj de pe data de %s",__DATE__);


	a.Load("E:\\test.txt");
	double value;
	for (int tr=0;tr<a.GetCount();tr++)
		printf("%s\n",a.Get(tr)->Name);

	a.Update("Vector",&x[0],sizeof(int)*10);

	//a.AddString("Value","Salut","Valoare Value");
	//a.AddBool("Ok",true);
	//a.AddDouble("DoubleValue",1.100);
	//a.AddAttribute("Vector",&x[0],GML::Utils::AttributeList::INT32,10);
	//a.Save("E:\\test.txt");

	return 0;
}

