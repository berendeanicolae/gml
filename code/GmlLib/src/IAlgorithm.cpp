#include "IAlgorithm.h"


struct ThreadLocalData
{
	GML::Algorithm::IAlgorithm	*me;
	HANDLE						*ptrhMainThread;
	GML::Utils::INotifier		*notif;
	GML::Utils::Timer			*algTimer;
};
DWORD WINAPI IAlgorithm_ThreadProc(LPVOID lpParameter)
{
	GML::Utils::GString		temp;
	ThreadLocalData *tld = (ThreadLocalData *)lpParameter;
	tld->algTimer->Start();
	tld->me->OnExecute();
	CloseHandle(*(tld->ptrhMainThread));	
	(*(tld->ptrhMainThread)) = NULL;
	tld->algTimer->Stop();
	tld->notif->Info("[%s] -> Total algorithm time: %s",tld->me->GetObjectName(),tld->algTimer->GetPeriodAsString(temp));
	temp.Distroy();
	// sterg obiectul
	delete tld;
	return 0;
}
//==============================================================================================
GML::Algorithm::IAlgorithm::IAlgorithm()
{
	notif = NULL;
	hMainThread = NULL;
	StopAlgorithm = false;
	LinkPropertyToUInt32("Command",Command,0,"!!LIST:None=0!!");
}

bool GML::Algorithm::IAlgorithm::Execute(char *command)
{
	ThreadLocalData				*tld;
	GML::Utils::AttributeList	attrCommand;


	if (notif==NULL)
		return false;
	if (command!=NULL)
	{
		notif->Info("[%s] -> Executing command: %s",ObjectName,command);
		if (attrCommand.AddString("Command",command)==false)
		{
			notif->Error("[%s] -> Unable to set command : %s !",ObjectName,command);
			return false;
		}
	}
	if (hMainThread!=NULL)
	{
		notif->Error("[%s] -> Already working. Unable to execute commnad %s !",ObjectName,command);
		return false;
	}
	if (command!=NULL)
	{
		if (SetProperty(attrCommand)==false)
		{
			notif->Error("[%s] -> Unable to set command property : %s !",ObjectName,command);
			return false;
		}
	}
	// altfel execut datele
	if ((tld = new ThreadLocalData())==NULL)
	{
		notif->Error("[%s] -> Internal error => Unable to create ThreadLocalData object",ObjectName);
		return false;
	}

	tld->me = this;
	tld->ptrhMainThread = &hMainThread;
	tld->algTimer = &algTimer;
	tld->notif = notif;
	
	// creez firul
	if ((hMainThread = CreateThread(NULL,0,IAlgorithm_ThreadProc,tld,CREATE_SUSPENDED,NULL))==NULL)
	{
		delete tld;
		notif->Error("[%s] -> Internal error => Unable to create working thread",ObjectName);
		return false;
	}
	ResumeThread(hMainThread);
	return true;
}
bool GML::Algorithm::IAlgorithm::Wait(UInt32 nrMiliseconds)
{
	if (hMainThread==NULL)
		return true;
	return (WaitForSingleObject(hMainThread,nrMiliseconds)==WAIT_OBJECT_0);
}
bool GML::Algorithm::IAlgorithm::Wait()
{
	return Wait(INFINITE);
}
