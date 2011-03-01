#include "IAlgorithm.h"

struct ThreadLocalData
{
	GML::Algorithm::IAlgorithm	*me;
	GML::Utils::GString			command;
	HANDLE						*ptrhMainThread;
};
DWORD WINAPI IAlgorithm_ThreadProc(LPVOID lpParameter)
{
	ThreadLocalData *tld = (ThreadLocalData *)lpParameter;
	tld->me->OnExecute(tld->command.GetText());
	CloseHandle(*(tld->ptrhMainThread));
	tld->command.Distroy();
	(*(tld->ptrhMainThread)) = NULL;
	// sterg obiectul
	delete tld;
	return 0;
}
//==============================================================================================
GML::Algorithm::IAlgorithm::IAlgorithm()
{
	notif = NULL;
	hMainThread = NULL;
}

bool GML::Algorithm::IAlgorithm::Execute(char *command)
{
	ThreadLocalData		*tld;

	if (notif==NULL)
		return false;
	if (hMainThread!=NULL)
	{
		notif->Error("Already working. Unable to execute commnad %s !",command);
		return false;
	}
	// altfel execut datele
	if ((tld = new ThreadLocalData())==NULL)
	{
		notif->Error("Internal error => Unable to create ThreadLocalData object");
		return false;
	}
	if (tld->command.Set(command)==false)
	{
		delete tld;
		notif->Error("Internal error => Unable to set string ... ");
		return false;
	}
	tld->me = this;
	tld->ptrhMainThread = &hMainThread;
	// creez firul
	if ((hMainThread = CreateThread(NULL,0,IAlgorithm_ThreadProc,tld,CREATE_SUSPENDED,NULL))==NULL)
	{
		delete tld;
		notif->Error("Internal error => Unable to create working thread");
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