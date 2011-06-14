// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "GraphicNotifier.h"

HWND parent = NULL;
DWORD nrWindows = 0;
HANDLE hEvent;


#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 500


#define WM_NEW_ALGORITHM (WM_USER+2)
#define WM_DELETE_ALGORITHM (WM_USER+3)


bool CreateMainWindow(GraphicNotifier* newGraphicNotifier)
{
	RECT desktopRect;
	CRect tabSize;
	HWND desktopWindow;

	desktopWindow = GetDesktopWindow();
	GetClientRect(desktopWindow,&desktopRect);


	desktopRect.right = (LONG)::GetSystemMetrics( SM_CXFULLSCREEN );
	desktopRect.bottom = (LONG)::GetSystemMetrics( SM_CYFULLSCREEN );

	tabSize.left = desktopRect.left/2-(WINDOW_WIDTH/2);
	tabSize.top = desktopRect.bottom /2 - (WINDOW_HEIGHT/2);
	tabSize.right = tabSize.left+ WINDOW_WIDTH;
	tabSize.bottom = tabSize.top+ WINDOW_HEIGHT;

	
	newGraphicNotifier->Create("Graphic Notifier",tabSize,NULL);

	return true;
}

DWORD WINAPI run_thread_window(LPVOID lpThreadParameter)
{
	GraphicNotifier* newGraphicNotifier;
	
	newGraphicNotifier = (GraphicNotifier*)lpThreadParameter;
	CreateMainWindow(newGraphicNotifier);
	SetEvent(hEvent);
	newGraphicNotifier->RunModalLoop();

	return NULL;
}

 extern "C"  __declspec(dllexport) void*  Init()
{
	char temp[100];
	CRect tabSize;

	HWND topWindow =   GetForegroundWindow();
	if(topWindow==NULL)
		return NULL;
	//AfxMessageBox("Got foreground window");
	if(!RealGetWindowClass(topWindow,temp,100))
		return NULL;
	if(strncmp(temp,"#32770",6) == 0)
	{
			//sunt in overlapped
			if(!IsWindow(parent))
			{
				parent = NULL;
			}

			if(parent == NULL)
			{
				parent = topWindow;
				//GetWindowText(parent,temp,100);
				//AfxMessageBox(temp);
			}
			::GetClientRect(parent,&tabSize);
			tabSize.left=+15;
			GraphicNotifier* newGraphicNotifier = new GraphicNotifier();
			newGraphicNotifier->Create("",tabSize,parent);
			nrWindows+=1;
			SendMessageA(parent,WM_NEW_ALGORITHM,NULL,(LPARAM)newGraphicNotifier);
			
			return newGraphicNotifier;
			//AfxMessageBox("message sent");

			
	}else
		if (strcmp(temp,"ConsoleWindowClass") == 0)
		{
			


			
			GraphicNotifier* newGraphicNotifier = new GraphicNotifier();
			HANDLE hthread;
				
			hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
			hthread = CreateThread(NULL,NULL,run_thread_window,newGraphicNotifier,NULL,NULL);
			WaitForSingleObject(hEvent,INFINITE);

			
			

			return newGraphicNotifier;
		}

	return NULL;
	
	
}
 extern "C" __declspec(dllexport) bool   Notify(void* context,unsigned int messageID,void *Data,unsigned int DataSize)
{
	GraphicNotifier* currentGraphicNotifier;

	currentGraphicNotifier= (GraphicNotifier*)context;
	GML::Utils::AlgorithmResult	*res = (GML::Utils::AlgorithmResult *)Data;	
	
	
	char *text = (char *)Data;

	if(currentGraphicNotifier == NULL)
		return false;
	if(!IsWindow(currentGraphicNotifier->GetSafeHwnd()))
		return false;
	if(currentGraphicNotifier->windowDestroyed == true)
		return false;
	
	// pentru AlgResult
	switch(messageID)
	{
	case 100:
		currentGraphicNotifier->InsertStatistics(res);
		
		//	printf("%4d|TP:%5d |TN:%5d |FN:%5d |FP:%5d |Se:%3.2lf|Sp:%3.2lf|Acc:%3.2lf|%s\n",(res->Iteration+1),(int)res->tp,(int)res->tn,(int)res->fn,(int)res->fp,res->se,res->sp,res->acc,res->time.GetPeriodAsString(tempStr));
		return true;
		break;
	case GML::Utils::INotifier::NOTIFY_ERROR:
	case GML::Utils::INotifier::NOTIFY_INFO:
		currentGraphicNotifier->InsertMessage(text,DataSize,messageID);
		break;
	}
	return true;
}

 extern "C" __declspec(dllexport) bool   Uninit(void* context)
{
	GraphicNotifier* currentGraphicNotifier;

	currentGraphicNotifier= (GraphicNotifier*)context;
	if(parent != NULL)
	{
		if(IsWindow(parent))
		{
			SendMessageA(parent,WM_DELETE_ALGORITHM,NULL,(LPARAM)context);
			nrWindows-=1;
			if(nrWindows == 0)
				parent = NULL;
		}
		
	}

	if(currentGraphicNotifier != NULL)
	{
		delete(currentGraphicNotifier);
	}

	

	return true;
}



