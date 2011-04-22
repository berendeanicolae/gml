#include "stdio.h"
#include "ConsoleNotifier.h"

ConsoleNotifier::ConsoleNotifier()
{
	ObjectName = "ConsoleNotifier";
	LinkPropertyToBool("UseColors",useColors,false,"Specifies if colors should be used when showing mesages");
}
void ConsoleNotifier::SetColor(unsigned char Fore, unsigned char Back)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),(Fore & 15)+(Back & 15)*16);
}
void ConsoleNotifier::SaveCursorCoord()
{
	CONSOLE_SCREEN_BUFFER_INFO      csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),&csbi);
	savedCursorCoord = csbi.dwCursorPosition;
}
void ConsoleNotifier::RestoreCursorCoord()
{
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),savedCursorCoord);
}
bool ConsoleNotifier::OnInit()
{	
	tempStr.Create();
	savedCursorCoord.X = -1;
	return true;
}
bool ConsoleNotifier::Uninit()
{
	return true;
}
void ConsoleNotifier::PrintText(char *text,unsigned char fore,unsigned char back,bool printCRLF)
{
	if (savedCursorCoord.X != -1)
	{
		printf("\n");
		savedCursorCoord.X = -1;
	}
	if (useColors)
		SetColor(fore,back);
	printf(text);
	if (printCRLF)
		printf("\n");
}
bool ConsoleNotifier::Notify(UInt32 messageID,void *Data,UInt32 DataSize)
{
	char							*text = (char *)Data;
	double							p;
	GML::Utils::AlgorithmResult		*res = (GML::Utils::AlgorithmResult *)Data;	
	SetColor(7,0);

	switch (messageID)
	{
		case GML::Utils::INotifier::NOTIFY_ERROR:
			PrintText(text,12,0,true);
			break;
		case GML::Utils::INotifier::NOTIFY_INFO:
			PrintText(text,11,0,true);
			break;
		case GML::Utils::INotifier::NOTIFY_START_PROCENT:
			PrintText(text,9,0,false);
			SaveCursorCoord();
			timer.Start();
			break;
		case GML::Utils::INotifier::NOTIFY_PROCENT:
			RestoreCursorCoord();
			if (useColors)
				SetColor(15,0);
			p = *(double *)Data;
			if (p<0) 
				p = 0;
			if (p>1)
				p = 1;
			p = p * 100;
			printf("[%.2lf%%]   ",p);
			timer.Stop();
			p*=1000;
			printf("ETA:%s  ",timer.EstimateETA(tempStr,(UInt32)(p),100000));
			break;
		case GML::Utils::INotifier::NOTIFY_END_PROCENT:
			printf("\n");
			savedCursorCoord.X = -1;
			break;
		case 100:
			if (useColors)
				SetColor(14,0);
			printf("%4d|TP:%5d |TN:%5d |FN:%5d |FP:%5d |Se:%3.2lf|Sp:%3.2lf|Acc:%3.2lf|%s\n",(res->Iteration+1),(int)res->tp,(int)res->tn,(int)res->fn,(int)res->fp,res->se,res->sp,res->acc,res->time.GetPeriodAsString(tempStr));
			break;
		default:
			tempStr.SetFormated("Unknown message ID = %d , DataSize = %d",messageID,DataSize);
			PrintText(tempStr.GetText(),7,12,true);
			break;
	}


	//printf("%4d => ",messageID);
	//// pentru AlgResult
	//if (messageID==100)
	//{
	//	if (useColors)
	//		SetColor(14,0);
	//	GML::Utils::AlgorithmResult	*res = (GML::Utils::AlgorithmResult *)Data;	
	//	printf("%4d|TP:%5d |TN:%5d |FN:%5d |FP:%5d |Se:%3.2lf|Sp:%3.2lf|Acc:%3.2lf|%s\n",(res->Iteration+1),(int)res->tp,(int)res->tn,(int)res->fn,(int)res->fp,res->se,res->sp,res->acc,res->time.GetPeriodAsString(tempStr));
	//	SetColor(7,0);
	//	return true;
	//}
	//if (useColors)
	//{
	//	switch (messageID)
	//	{
	//		case GML::Utils::INotifier::NOTIFY_ERROR:
	//			SetColor(12,0);
	//			break;
	//		case GML::Utils::INotifier::NOTIFY_INFO:
	//			SetColor(11,0);
	//			break;
	//		case GML::Utils::INotifier::NOTIFY_START_PROCENT:
	//			SetColor(9,0);
	//			break;
	//	};
	//}
	//
	//// afisare date
	//if ((Data!=NULL) && (DataSize>0))
	//{
	//	for (UInt32 tr =0;tr<DataSize;tr++)
	//	{
	//		if ((text[tr]>=' ') && (text[tr]<=128))
	//			printf("%c",text[tr]);
	//		else
	//			printf("{\\x%02X}",(unsigned char)text[tr]);
	//	}
	//}
	//printf("\n");
	return true;
}
