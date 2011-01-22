#include "StdAfx.h"
#include "ThreadParalelUnit.h"

DWORD WINAPI ThreadParalelUnit_ThreadProc(LPVOID lpParameter)
{
	GML::Utils::ThreadParalelUnit *tpu = (GML::Utils::ThreadParalelUnit *)lpParameter;
	tpu->DoAction();
	return 0;
}
//===============================================================================================
GML::Utils::ThreadParalelUnit::ThreadParalelUnit(void)
{
	eventWorking = CreateEvent(NULL,TRUE,FALSE,NULL);
	eventWaiting = CreateEvent(NULL,TRUE,TRUE,NULL);
	hThread = CreateThread(NULL,0,ThreadParalelUnit_ThreadProc,this,0,NULL);
}

GML::Utils::ThreadParalelUnit::~ThreadParalelUnit(void)
{
	Execute(-1);
	Sleep(500);
	if (eventWorking!=NULL)
		CloseHandle(eventWorking);
	if (eventWaiting!=NULL)
		CloseHandle(eventWaiting);
	if (hThread!=NULL)
		CloseHandle(hThread);
	eventWorking = eventWaiting = NULL;
	hThread = NULL;
}
bool GML::Utils::ThreadParalelUnit::WaitToFinish()
{
	if (eventWorking==NULL)
		return false;
	if (WaitForSingleObject(eventWorking,INFINITE)!=WAIT_OBJECT_0)
		return true;
	return false;
}
void GML::Utils::ThreadParalelUnit::DoAction()
{
	while (true)
	{
		if (WaitForSingleObject(eventWaiting,INFINITE)!=WAIT_OBJECT_0)
			break;
		if (codeID==-1)
			break;
		fnThread(this,context);
		ResetEvent(eventWaiting);
		SetEvent(eventWorking);
	}
}
bool GML::Utils::ThreadParalelUnit::Execute(unsigned int _codeID)
{
	if ((fnThread==NULL) || (eventWorking==NULL) || (eventWaiting==NULL))
		return false;
	codeID = _codeID;
	ResetEvent(eventWorking);

	SetEvent(eventWaiting);
	return true;
}