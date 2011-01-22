// TestGmlLib.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "gmllib.h"



int _tmain(int argc, _TCHAR* argv[])
{
	GML::Utils::Timer	t;

	t.Start();
	for (int tr=0;tr<100000;tr++)
	{
		for (int gr=0;gr<10000;gr++)
		{
			_asm nop;
		}
	}
	t.Stop();
	printf("ETA = [%s]\n",t.GetPeriodAsString());
	return 0;
}

