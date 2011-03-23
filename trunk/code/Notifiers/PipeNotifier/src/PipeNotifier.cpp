#include "stdio.h"
#include "PipeNotifier.h"

PipeNotifier::PipeNotifier()
{
	ObjectName = "PipeNotifier";

	memset(&pi,0,sizeof(pi));
	hPipe = INVALID_HANDLE_VALUE;

	LinkPropertyToString("Client",Client,"","Specifies the name of the executable that will receive the pipe messages !");
	LinkPropertyToBool  ("CreateNewConsole",CreateNewConsole,false,"Specify if a new console is needed for the Client application !");
}

bool PipeNotifier::OnInit()
{	
    STARTUPINFO				si;
	GML::Utils::GString		str;
	DWORD					attr;
    
	memset(&si,0,sizeof(si));
	memset(&pi,0,sizeof(pi));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;

	if (Client.Len()==0)
	{
		DEBUGMSG("[%s] -> Client was not specified !",ObjectName);
		return false;
	}

	attr = CREATE_SUSPENDED;
	if (CreateNewConsole)
		attr |= CREATE_NEW_CONSOLE;

	if (!CreateProcess(Client.GetText(),NULL,NULL,NULL,FALSE,attr,NULL,NULL,&si,&pi))
	{
		DEBUGMSG("[%s] -> Unable to run %s !",ObjectName,Client.GetText());
		memset(&pi,0,sizeof(pi));
		return false;
	}
	
	// creez pipe-ul
	while (true)
	{
		if (str.SetFormated("\\\\.\\pipe\\gml_%08X",pi.dwProcessId)==false)
			break;
		if ((hPipe = CreateNamedPipe(str.GetText(),PIPE_ACCESS_DUPLEX,PIPE_TYPE_BYTE|PIPE_READMODE_BYTE|PIPE_WAIT,PIPE_UNLIMITED_INSTANCES,2048,2048,0,NULL))==INVALID_HANDLE_VALUE)
			break;
		// dau drumul la process
		ResumeThread(pi.hThread);
		if (!ConnectNamedPipe(hPipe, NULL))
		{
			DWORD error = GetLastError();
			DEBUGMSG("[%s] -> GetLastError : %d \n",ObjectName,error);
			break;
		}

		return true;
	}
	
	if (hPipe!=INVALID_HANDLE_VALUE)
		CloseHandle(hPipe);
	DEBUGMSG("[%s] -> Unable to run/conect to %s !",ObjectName,Client.GetText());
	TerminateProcess(pi.hProcess,1);
	memset(&pi,0,sizeof(pi));
	return false;
}
bool PipeNotifier::Uninit()
{
	if (pi.hProcess!=NULL)
		TerminateProcess(pi.hProcess,0);
	if (hPipe!=INVALID_HANDLE_VALUE)
		CloseHandle(hPipe);	

	memset(&pi,0,sizeof(pi));
	hPipe = INVALID_HANDLE_VALUE;
	return true;
}
bool PipeNotifier::Notify(UInt32 messageID,void *Data,UInt32 DataSize)
{
	DWORD			nrWrite;
	//unsigned char	temp[2048];

	if (hPipe!=INVALID_HANDLE_VALUE)
	{
		//memcpy(&temp[8],Data,DataSize);
		//*(UInt32 *)&temp[0] = DataSize+8;
		//*(UInt32 *)&temp[4] = messageID;
		//if ((WriteFile(hPipe,temp,DataSize+8,&nrWrite,NULL)==FALSE) || (nrWrite!=DataSize+8))
		//	return false;
		


		DataSize+=sizeof(UInt32)*2;
		if ((WriteFile(hPipe,&DataSize,sizeof(UInt32),&nrWrite,NULL)==FALSE) || (nrWrite!=sizeof(UInt32)))
			return false;
		if ((WriteFile(hPipe,&messageID,sizeof(UInt32),&nrWrite,NULL)==FALSE) || (nrWrite!=sizeof(UInt32)))
			return false;
		if ((WriteFile(hPipe,Data,DataSize,&nrWrite,NULL)==FALSE) || (nrWrite!=DataSize))
			return false;
		//printf("Sending message ID: %d [%d octeti]\n",messageID,DataSize);
		return true;
	}
	
	return false;
}
