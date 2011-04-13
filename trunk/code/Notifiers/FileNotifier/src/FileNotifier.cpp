#include "stdio.h"
#include "FileNotifier.h"

FileNotifier::FileNotifier()
{
	ObjectName = "FileNotifier";
	LinkPropertyToBool  ("FlushAfterEachWrite",flushAfterEachWrite,false,"Is set , the data will be flush to file after each notification message. Notifications will be slower !");
	LinkPropertyToString("FileName",fileName,"","Name of the file where the notifications will be written ");
	LinkPropertyToUInt32("TimeFormat",showNotificationTime,SHOW_TIME_NONE,"!!LIST:None=0,DateTime,Miliseconds!!\nSpecify how time will be written");
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

	switch (showNotificationTime)
	{
		case SHOW_TIME_NONE:
			tmp.Set("");
			break;
		case SHOW_TIME_DATETIME:
			GetSystemTime(&tm);
			tmp.SetFormated("%04d.%02d.%02d  %02d:%02d:%02d.%03d ",tm.wYear,tm.wMonth,tm.wDay,tm.wHour,tm.wMinute,tm.wSecond,tm.wMilliseconds);
			break;
		case SHOW_TIME_MILISECONDS:
			tmp.SetFormated("%10u ",GetTickCount());
			break;
	}

	switch (messageID)
	{
		case GML::Utils::INotifier::NOTIFY_ERROR:
			tmp.AddFormated("[ERROR] %s\n",text);
			break;
		case GML::Utils::INotifier::NOTIFY_INFO:
			tmp.AddFormated("[INFOS] %s\n",text);
			break;
		case 100:
			res = (GML::Utils::AlgorithmResult *)Data;	
			tmp.AddFormated("[ RES ] %4d|TP:%5d |TN:%5d |FN:%5d |FP:%5d |Se:%3.2lf|Sp:%3.2lf|Acc:%3.2lf|%s\n",(res->Iteration+1),(int)res->tp,(int)res->tn,(int)res->fn,(int)res->fp,res->se,res->sp,res->acc,res->time.GetPeriodAsString(tempStr));
			break;
	};
	if (file.Write(tmp.GetText(),tmp.Len())==false)
		return false;
	if (flushAfterEachWrite)
		file.Flush();

	return true;
}
