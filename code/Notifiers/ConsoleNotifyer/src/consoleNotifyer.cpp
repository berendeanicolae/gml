#include "stdio.h"
#include "ConsoleNotifyer.h"

ConsoleNotifyer::ConsoleNotifyer()
{
	LinkBool("UseColors",useColors,false,"Specifies if colors should be used when showing mesages");
}
void ConsoleNotifyer::SetColor(unsigned char Fore, unsigned char Back)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),(Fore & 15)+(Back & 15)*16);
}
bool ConsoleNotifyer::OnInit()
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
	SetColor(7,0);
	printf("%4d => ",messageID);
	// pentru AlgResult
	if (messageID==100)
	{
		if (useColors)
			SetColor(14,0);
		GML::Utils::AlgorithmResult	*res = (GML::Utils::AlgorithmResult *)Data;
		printf("TP:%5d |TN:%5d |FN:%5d |FP:%5d |Se:%3.2lf|Sp:%3.2lf|Acc:%3.2lf|\n",(int)res->tp,(int)res->tn,(int)res->fn,(int)res->fp,res->se,res->sp,res->acc);
		SetColor(7,0);
		return true;
	}
	if (useColors)
	{
		switch (messageID)
		{
			case GML::Utils::INotify::NOTIFY_ERROR:
				SetColor(10,0);
				break;
			case GML::Utils::INotify::NOTIFY_INFO:
				SetColor(11,0);
				break;
		};
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
	SetColor(7,0);
	return true;
}
