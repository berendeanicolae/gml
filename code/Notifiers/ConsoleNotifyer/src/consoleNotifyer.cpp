#include "stdio.h"
#include "ConsoleNotifyer.h"


bool ConsoleNotifyer::Init(void *initData)
{
	return true;
}
bool ConsoleNotifyer::Uninit()
{
	return true;
}
bool ConsoleNotifyer::Notify(UInt32 messageID,void *Data,UInt32 DataSize)
{
	char *text = (char *)Data;
	printf("%4d => ",messageID);
	// pentru AlgResult
	if (messageID==100)
	{
		GML::Utils::AlgorithmResult	*res = (GML::Utils::AlgorithmResult *)Data;
		printf("TP:%5d |TN:%5d |FN:%5d |FP:%5d |Se:%3.2lf|Sp:%3.2lf|Acc:%3.2lf|\n",(int)res->tp,(int)res->tn,(int)res->fn,(int)res->fp,res->se,res->sp,res->acc);
		return true;
	}
	if ((Data!=NULL) && (DataSize>0))
	{
		for (UInt32 tr =0;tr<DataSize;tr++)
		{
			if ((text[tr]>=' ') && (text[tr]<=128))
				printf("%c",text[tr]);
			else
				printf("{\\x%02X}",(unsigned char)text[tr]);
		}
	}
	printf("\n");
	return true;
}
