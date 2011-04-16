#include "stdio.h"
#include "FileNotifier.h"

FileNotifier::FileNotifier()
{
	ObjectName = "FileNotifier";
	LinkPropertyToBool  ("FlushAfterEachWrite",flushAfterEachWrite,false,"Is set , the data will be flush to file after each notification message. Notifications will be slower !");
	LinkPropertyToString("FileName",fileName,"","Name of the file where the notifications will be written ");
	LinkPropertyToUInt32("TimeFormat",showNotificationTime,SHOW_TIME_NONE,"!!LIST:None=0,DateTime,Miliseconds!!\nSpecify how time will be written");
	LinkPropertyToBool  ("ParsableFormat",parsableFormat,false,"Specify if the file format should be parsable ('|' separated columns)");
}
bool FileNotifier::OnInit()
{	
	return file.Create(fileName.GetText(),true);
}
bool FileNotifier::Uninit()
{
	file.Close();
	return true;
}
bool FileNotifier::Notify(UInt32 messageID,void *Data,UInt32 DataSize)
{
	GML::Utils::GString				tmp,tempStr;
	char							*text = (char *)Data;
	GML::Utils::AlgorithmResult		*res;
	SYSTEMTIME						tm;
	double							*p = (double *)Data;

	switch (showNotificationTime)
	{
		case SHOW_TIME_NONE:
			if (parsableFormat)
				tmp.Set("|");
			else
				tmp.Set("");
			break;
		case SHOW_TIME_DATETIME:
			GetSystemTime(&tm);
			if (parsableFormat)
				tmp.SetFormated("%04d.%02d.%02d  %02d:%02d:%02d.%03d|",tm.wYear,tm.wMonth,tm.wDay,tm.wHour,tm.wMinute,tm.wSecond,tm.wMilliseconds);
			else
				tmp.SetFormated("%04d.%02d.%02d  %02d:%02d:%02d.%03d ",tm.wYear,tm.wMonth,tm.wDay,tm.wHour,tm.wMinute,tm.wSecond,tm.wMilliseconds);
			break;
		case SHOW_TIME_MILISECONDS:
			if (parsableFormat)
				tmp.SetFormated("%10u|",GetTickCount());
			else
				tmp.SetFormated("%10u ",GetTickCount());
			break;
	}

	switch (messageID)
	{
		case GML::Utils::INotifier::NOTIFY_ERROR:
			if (parsableFormat)
				tmp.AddFormated("ERROR|%s\n",text);
			else
				tmp.AddFormated("[ERROR] %s\n",text);
			break;
		case GML::Utils::INotifier::NOTIFY_INFO:
			if (parsableFormat)
				tmp.AddFormated("INFO|%s\n",text);
			else
				tmp.AddFormated("[INFOS] %s\n",text);
			break;
		case GML::Utils::INotifier::NOTIFY_START_PROCENT:
			if (parsableFormat)
				tmp.AddFormated("PROCENT|%s|[",text);
			else
				tmp.AddFormated("[PROC ] %s [",text);
			lastProcValue = 0;
			break;
		case GML::Utils::INotifier::NOTIFY_END_PROCENT:
			tmp.Set("]\n");
			break;
		case GML::Utils::INotifier::NOTIFY_PROCENT:
			if ((*p)>=lastProcValue)
			{
				tmp.Set(".");
				lastProcValue+=0.1;
			} else {
				// nu e nevoie sa mai scriu ceva
				return true;
			}
			break;
		case 100:
			res = (GML::Utils::AlgorithmResult *)Data;	
			if (parsableFormat)
			{
				if (res->Iteration==0)
				{
					tempStr.Set(&tmp);
					tmp.AddFormated("RESULT_HEADER|ITERATION|TP|TN|FN|FP|Se|Sp|Acc|Time\n%s",tempStr.GetText());					
				}
				tmp.AddFormated("RESULTS|%d|%d|%d|%d|%d|%lf|%lf|%lf|%s\n",(res->Iteration+1),(int)res->tp,(int)res->tn,(int)res->fn,(int)res->fp,res->se,res->sp,res->acc,res->time.GetPeriodAsString(tempStr));
			} else {
				tmp.AddFormated("[ RES ] %4d|TP:%5d |TN:%5d |FN:%5d |FP:%5d |Se:%3.2lf|Sp:%3.2lf|Acc:%3.2lf|%s\n",(res->Iteration+1),(int)res->tp,(int)res->tn,(int)res->fn,(int)res->fp,res->se,res->sp,res->acc,res->time.GetPeriodAsString(tempStr));
			}
			break;
	};
	if (file.Write(tmp.GetText(),tmp.Len())==false)
		return false;
	if (flushAfterEachWrite)
		file.Flush();

	return true;
}
