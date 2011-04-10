#include "StdAfx.h"
#include "GraphicNotifierContainer.h"


GraphicNotifierContainer::GraphicNotifierContainer(void)
{
	
	nrAlgorithms = 0;
	
	someBrush.CreateSolidBrush(RGB(100,195,195));  
	fnt.CreateFont(12,0,0,0,FW_THIN,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,DEFAULT_PITCH+FF_DONTCARE,TEXT("Arial"));
	parentWindow = NULL;

	//temp
	hModule = LoadLibrary("C:\\Users\\BMR\\Documents\\Visual Studio 2010\\Projects\\interfaceNotifier\\Debug\\interfacenotifier.dll");
	

}

BEGIN_MESSAGE_MAP(GraphicNotifierContainer, CWnd)
	ON_WM_CLOSE()
	ON_NOTIFY(TCN_SELCHANGE, ALGORITHM_TAB_CONTROL, OnTcnSelchange)
	ON_MESSAGE(WM_NEW_ALGORITHM,&GraphicNotifierContainer::OnNewAlgorithm)
	ON_MESSAGE(WM_DELETE_ALGORITHM,&GraphicNotifierContainer::OnCloseAlgorithm)
	
END_MESSAGE_MAP()



void GraphicNotifierContainer::Create(TCHAR* text,int x,int y,int width,int height,CWnd *parent, UINT ID)
{
	this->width = width;
	this->height = height;
	this->x = x;
	this->y = y;
	this->parentWindow = parent;
	DWORD windowStyle;
	

	windowStyle = WS_OVERLAPPED | WS_VISIBLE | WS_SYSMENU;
	CWnd::CreateEx(WS_EX_OVERLAPPEDWINDOW,"#32770","Graphical Notifier: Algorithms",windowStyle,CRect(x,y,x+width,y+height),parent, NULL);
	
	
	CSize size;
	currentTabControl.Create(TCS_TABS | TCS_FIXEDWIDTH | WS_CHILD | WS_VISIBLE|TCS_MULTILINE | TCS_VERTICAL,CRect(0,0,this->width,this->height),this, ID);
	currentTabControl.SetFont(&fnt);

	


	//doar temporar
/*
	TCITEM tcItem;
	tcItem.mask = TCIF_TEXT;
	tcItem.pszText = _T("Algorithm1");
	currentTabControl.InsertItem(0, &tcItem);
	tcItem.pszText = _T("Algorithm2");
	currentTabControl.InsertItem(1, &tcItem);
	tcItem.pszText = _T("Algorithm3");
	currentTabControl.InsertItem(2, &tcItem);
*/
}
GraphicNotifierContainer::~GraphicNotifierContainer(void)
{
}


void GraphicNotifierContainer::OnClose()
{
	
	
/*temporar	
	typedef bool ( *UninitFunction)(void* arg);
	UninitFunction temp;

	temp = (UninitFunction)GetProcAddress(hModule,"Uninit");

	for(int i=0;i<nrAlgorithms;i++)
		temp(algorithmObjects[i]);
//-*/	
	this->DestroyWindow();
	if(parentWindow != NULL)
		::SendMessageA(parentWindow->GetSafeHwnd(),WM_ALGORITHM_CONTAINER_CLOSED,NULL,NULL);


	
	


}

CWnd* GraphicNotifierContainer::AddAlgorithm(char* algorithmName)
{
	TCITEM tcItem;
	int currentTabId;
	CRect tabNameSize;
	CRect tabSize;


	if(nrAlgorithms == MAX_ALGORITHMS)
		return NULL;

	tcItem.mask = TCIF_TEXT;
	tcItem.pszText = algorithmName;

	currentTabId = currentTabControl.InsertItem(nrAlgorithms,&tcItem);
	if (currentTabId ==-1)
		return NULL;
	
	nrAlgorithms+=1;
	

	/*
//+ temporar

	typedef void* ( *InitFunction)();
	InitFunction temp;

	temp = (InitFunction)GetProcAddress(hModule,"Init");

	temp();
//- temporar
*/



}

void GraphicNotifierContainer::OnTcnSelchange(NMHDR *pNMHDR, LRESULT *pResult)
{
	int currentTabId;
	currentTabId = currentTabControl.GetCurSel();
	if (currentTabId != -1)
	{
		for(int i =0;i<nrAlgorithms;i++)
			if(currentTabId == i)
			{
				algorithmObjects[currentTabId]->ShowWindow(SW_SHOW);
			}
			else
			{
				algorithmObjects[i]->ShowWindow(SW_HIDE);
			}
	}

	*pResult = 0;
}


LRESULT GraphicNotifierContainer::OnNewAlgorithm(WPARAM p1, LPARAM p2)
{
	algorithmObjects[nrAlgorithms-1] = (GraphicNotifier*)p2;

	return NULL;
}
LRESULT GraphicNotifierContainer::OnCloseAlgorithm(WPARAM p1, LPARAM p2)
{
	GraphicNotifier* temp;
	int j = 0;

	temp = (GraphicNotifier*)p2;
	if(temp == NULL)
		return 0;

	for(int i=0;i<nrAlgorithms;i++)
		if(algorithmObjects[i] == temp)
		{
			j = i;
			break;
		}
	for(;j<nrAlgorithms-1;j++)
	{
		algorithmObjects[j] = algorithmObjects[j+1];
	}
	algorithmObjects[nrAlgorithms--] = NULL;
	return NULL;
}