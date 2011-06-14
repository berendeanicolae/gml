#include "StdAfx.h"
#include "GraphicNotifier.h"


GraphicNotifier::GraphicNotifier(void)
{
	nrObjects = 0;
}


void GraphicNotifier::Create(TCHAR* text,CRect& rect,CWnd *parent)
{
	TCITEM tcItem;
	DWORD windowStyle;

	this->width = width;
	this->height = height;
	this->x = x;
	this->y = y;



	if(parent == NULL)
	{
		windowStyle = WS_OVERLAPPED | WS_VISIBLE | WS_SYSMENU;
		CWnd::CreateEx(WS_EX_OVERLAPPEDWINDOW,"#32770","Graphical Notifier",windowStyle,rect,parent, NULL);
	}
	else
	{
		windowStyle= WS_OVERLAPPED |WS_VISIBLE;
		CWnd::Create(NULL,"Graphical Notifier",windowStyle,rect,parent,NULL);
	}



	someBrush.CreateSolidBrush(RGB(100,195,195));  
	fnt.CreateFont(12,0,0,0,FW_THIN,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,DEFAULT_PITCH+FF_DONTCARE,TEXT("Arial"));


	//creaza tab controlul
	tabControl.Create(TCS_TABS | TCS_FIXEDWIDTH | WS_CHILD | WS_VISIBLE|TCS_MULTILINE ,rect,this, TAB_CONTROL);
	tabControl.SetFont(&fnt);


	
	//creaza taburile
	tcItem.mask = TCIF_TEXT;
	tcItem.pszText = _T("Statistics");
	tabControl.InsertItem(0, &tcItem);
	tcItem.pszText = _T("Temp View");
	tabControl.InsertItem(1, &tcItem);
	tcItem.pszText = _T("Graphic View");
	tabControl.InsertItem(2, &tcItem);

	temp.Create(text,WS_VISIBLE,CRect(30,30,300,60),this);
	AddObject(&temp,1);

	

	lstAlgResult.Create(LVS_ALIGNLEFT |WS_BORDER |WS_EX_STATICEDGE|SS_SUNKEN| LVS_REPORT  | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_VISIBLE,CRect(TAB_LEFT,TAB_TOP,TAB_RIGHT,TAB_BOTTOM),this,ID_RESULT_LIST);


	lstAlgResult.InsertColumn(0,"test1",LVCFMT_CENTER,100);
	lstAlgResult.InsertColumn(1,"test2",LVCFMT_CENTER,100);
	lstAlgResult.InsertColumn(2,"test3",LVCFMT_CENTER,100);
	AddObject(&lstAlgResult,0);

	//fortez redesenarea
	OnTcnSelchange(NULL,NULL);

	
}

void GraphicNotifier::Create(TCHAR* text,int x,int y,int width,int height,CWnd *parent)
{
	CRect currentRect(x,y,x+width,y+height);
	Create(text,currentRect,parent);
}


BEGIN_MESSAGE_MAP(GraphicNotifier, CWnd)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_NOTIFY(TCN_SELCHANGE, TAB_CONTROL, OnTcnSelchange)
END_MESSAGE_MAP()


GraphicNotifier::~GraphicNotifier(void)
{
}


void GraphicNotifier::OnPaint()
{

	CPaintDC dc(this);


	int res = dc.SaveDC();
	dc.FillRect(CRect(0,0,this->width,this->height),&someBrush);
	
	dc.RestoreDC(res);	
	//CWnd::OnPaint();

}

bool GraphicNotifier::AddObject(CWnd* object, unsigned int tabId)
{
	if (tabId < 0 || tabId >= NR_TABS)
		return false;
	if(object == NULL)
		return false;
	if(nrObjects >= MAX_CONTROLS)
		return false;

	tabControlObjects[nrObjects].object = object;
	tabControlObjects[nrObjects++].tabId = tabId;

	return true;


}

void GraphicNotifier::OnClose()
{
	this->DestroyWindow();
}



void GraphicNotifier::OnTcnSelchange(NMHDR *pNMHDR, LRESULT *pResult)
{
	int currentTabId;
	currentTabId = tabControl.GetCurSel();
	if (currentTabId != -1)
	{
		for(int i =0;i<nrObjects;i++)
			if(currentTabId == tabControlObjects[i].tabId)
			{
				tabControlObjects[i].object->ShowWindow(SW_SHOW);
			}
			else
			{
				tabControlObjects[i].object->ShowWindow(SW_HIDE);
			}
	}

}