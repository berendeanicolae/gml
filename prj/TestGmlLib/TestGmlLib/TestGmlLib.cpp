// TestGmlLib.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "gmllib.h"

class Test
{
public:
	int x,y,z,t,a,b,c;
};

Test& Get()
{
	Test *t = new Test();
	t->x = t->y = 10;
	return (*t);
}

int _tmain(int argc, _TCHAR* argv[])
{
	Test t = Get();

	GML::Utils::File	f;
	GML::Utils::GString	str;

	f.OpenRead("E:\\test.txt");
	while (f.ReadNextLine(str))
	{
		printf("%s\n",str.GetText());
	}


	int x[10]; //= { 1,5,2,6,3,4,8,9,0,11 };
	GML::Utils::AttributeList	a;

	a.Create("Name='database';value=-20;action=true;learningRate=0.05;iteratii=100;string=Ce mai faci zilele astea;contor=100");
	a.Save("E:\\test2.txt");



	GML::Utils::INotify *notif = GML::Builder::CreateNotifyer("E:\\lucru\\GML\\gml\\lib\\ConsoleNotifyer.ntf");
	if (notif)
		notif->NotifyString(0,"Mesaj de pe data de %s",__DATE__);


	a.Load("E:\\test.txt");
	double value;
	a.Update("DoubleValue",&value,sizeof(value));
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

