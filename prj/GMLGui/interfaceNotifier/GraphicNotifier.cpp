#include "StdAfx.h"
#include "..\interfaceNotifier\GraphicNotifier.h"


GraphicNotifier::GraphicNotifier(void)
{
	nrObjects = 0;
	windowDestroyed = false;
	lastProgressBar = 0;
	statisticsCreated = false;
}


void GraphicNotifier::Create(TCHAR* text,CRect& rect,HWND parent)
{
	TCITEM tcItem;
	DWORD windowStyle;

	this->width = rect.Width();
	this->height = rect.Height();
	this->x = rect.left;
	this->y = rect.top;



	if(parent == NULL)
	{
		windowStyle = WS_OVERLAPPED | WS_VISIBLE | WS_SYSMENU;
		CWnd::CreateEx(WS_EX_OVERLAPPEDWINDOW,"#32770","Graphical Notifier",windowStyle,rect.left,rect.top,rect.Width(),rect.Height(),parent, NULL);
		CenterWindow();
		
		
	}
	else
	{
		windowStyle= WS_CHILD |WS_VISIBLE;
		CWnd::CreateEx(NULL,NULL,"Graphical Notifier",windowStyle,rect.left,rect.top,rect.Width(),rect.Height(),parent, NULL);
	}

	someBrush.CreateSolidBrush(RGB(100,195,195));  
	fnt.CreateFont(12,0,0,0,FW_THIN,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,DEFAULT_PITCH+FF_DONTCARE,TEXT("Arial"));


	//creaza tab controlul
	tabControl.Create(TCS_TABS | TCS_FIXEDWIDTH | WS_CHILD | WS_VISIBLE|TCS_MULTILINE ,CRect(0,0,this->width,this->height),this, TAB_CONTROL);
	tabControl.SetFont(&fnt);


	//creaza taburile
	tcItem.mask = TCIF_TEXT;
	tcItem.pszText = _T("Status");
	tabControl.InsertItem(0, &tcItem);
	
	lstStatus.CreateEx(LVS_EX_GRIDLINES|WS_EX_STATICEDGE,WS_BORDER| LVS_REPORT  | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_VISIBLE | LVS_OWNERDRAWFIXED ,CRect(TAB_LEFT,TAB_TOP,TAB_RIGHT,TAB_BOTTOM-100),&tabControl,ID_STATUS_LIST);
	lstStatus.SetFont(&fnt);

	//	printf("%4d|TP:%5d |TN:%5d |FN:%5d |FP:%5d |Se:%3.2lf|Sp:%3.2lf|Acc:%3.2lf|%s\n",(res->Iteration+1),(int)res->tp,(int)res->tn,(int)res->fn,(int)res->fp,res->se,res->sp,res->acc,res->time.GetPeriodAsString(tempStr));
	int count = 0;
	int nrColomns=2;

	lstStatus.InsertColumn(count++,"Tile",LVCFMT_LEFT,200);
	lstStatus.InsertColumn(count++,"Details",LVCFMT_LEFT,TAB_WIDTH -220);
	AddObject(&lstStatus,0);
	
	//CreateStatisticsTab();

	//fortez redesenarea
	//OnTcnSelchange(NULL,NULL);

	windowDestroyed = false;


	
}

void GraphicNotifier::Create(TCHAR* text,int x,int y,int width,int height,HWND parent)
{
	CRect currentRect(x,y,x+width,y+height);
	Create(text,currentRect,parent);
}


BEGIN_MESSAGE_MAP(GraphicNotifier, CWnd)

	ON_WM_CLOSE()
	ON_NOTIFY(TCN_SELCHANGE, TAB_CONTROL, OnTcnSelchange)
	ON_MESSAGE(WM_ALGORITHM_NOTIFY_RESULT,&GraphicNotifier::OnNotifyResult)
END_MESSAGE_MAP()


GraphicNotifier::~GraphicNotifier(void)
{
}



bool GraphicNotifier::SetProgress(double procent)
{
	
	
	return  lstStatus.SetProgress(lastProgressBar,(int)(procent));
	
}

bool GraphicNotifier::CreateStatisticsTab()
{
	int count = 0;
	int nrColomns=9;
	TCITEM tcItem;
	
	tcItem.mask = TCIF_TEXT;
	tcItem.pszText = _T("Statistics");
	if(tabControl.InsertItem(1, &tcItem)==-1)
		return false;
	this->UpdateWindow();
	tabControl.UpdateWindow();
	
	if(!lstAlgResult.CreateEx(LVS_EX_GRIDLINES|WS_EX_STATICEDGE,WS_BORDER | LVS_REPORT  | LVS_SINGLESEL | LVS_SHOWSELALWAYS  ,CRect(TAB_LEFT,TAB_TOP,TAB_RIGHT,TAB_BOTTOM-10),this,ID_RESULT_LIST))
		return false;
	statisticsCreated = true;
	lstAlgResult.SetFont(&fnt);
	tabControl.UpdateWindow();
	lstAlgResult.UpdateWindow();
	
	lstAlgResult.InsertColumn(count++,"Nr",LVCFMT_CENTER,(TAB_WIDTH /nrColomns)/2);
	lstAlgResult.InsertColumn(count++,"TP",LVCFMT_CENTER,TAB_WIDTH /nrColomns);
	lstAlgResult.InsertColumn(count++,"TN",LVCFMT_CENTER,TAB_WIDTH /nrColomns);
	lstAlgResult.InsertColumn(count++,"FN",LVCFMT_CENTER,TAB_WIDTH /nrColomns);
	lstAlgResult.InsertColumn(count++,"FP",LVCFMT_CENTER,TAB_WIDTH /nrColomns);
	lstAlgResult.InsertColumn(count++,"Se",LVCFMT_CENTER,TAB_WIDTH /nrColomns);
	lstAlgResult.InsertColumn(count++,"Sp",LVCFMT_CENTER,TAB_WIDTH /nrColomns);
	lstAlgResult.InsertColumn(count++,"Acc",LVCFMT_CENTER,TAB_WIDTH /nrColomns);
	lstAlgResult.InsertColumn(count++,"Time",LVCFMT_LEFT,(TAB_WIDTH /nrColomns)*1.5);
	
	//printf("%4d|TP:%5d |TN:%5d |FN:%5d |FP:%5d |Se:%3.2lf|Sp:%3.2lf|Acc:%3.2lf|Med:%3.2lf|%s\n",(res->Iteration+1),(int)res->tp,(int)res->tn,(int)res->fn,(int)res->fp,res->se,res->sp,res->acc,res->med,res->time.GetPeriodAsString(tempStr));
	AddObject(&lstAlgResult,1);
	//lstAlgResult.
	//lstAlgResult.InitControlContainer
	this->RedrawWindow();

	return true;

}

LRESULT GraphicNotifier::OnNotifyResult(WPARAM p1, LPARAM p2)
{
	GML::Utils::AlgorithmResult	*res = (GML::Utils::AlgorithmResult *)p1;	

	GraphicNotifier* currentGraphicNotifier = (GraphicNotifier* )p2;
	currentGraphicNotifier->InsertStatistics(res);
	return 1;
}
bool GraphicNotifier::InsertStatistics(GML::Utils::AlgorithmResult	*res )
{
	GML::Utils::GString		tempStr;
	char temp[100];
	int result;
	int count = 0;
	bool visibile;

	
	if(!statisticsCreated)
		CreateStatisticsTab();
	
	sprintf(temp,"%d",res->Iteration+1);
	
	visibile = lstAlgResult.IsWindowVisible();

	result = lstAlgResult.InsertItem(res->Iteration,temp);
	lstAlgResult.SetItemText(result,count++,temp);
	sprintf(temp,"%5d",(int)res->tp);
	lstAlgResult.SetItemText(result,count++,temp);

	sprintf(temp,"%5d",(int)res->tn);
	lstAlgResult.SetItemText(result,count++,temp);

	sprintf(temp,"%5d",(int)res->fn);
	lstAlgResult.SetItemText(result,count++,temp);

	sprintf(temp,"%5d",(int)res->fp);
	lstAlgResult.SetItemText(result,count++,temp);

	sprintf(temp,"%3.2lf",res->se);
	lstAlgResult.SetItemText(result,count++,temp);

	sprintf(temp,"%3.2lf",res->sp);
	lstAlgResult.SetItemText(result,count++,temp);

	sprintf(temp,"%3.2lf",res->acc);
	lstAlgResult.SetItemText(result,count++,temp);

	sprintf(temp,"%s",res->time.GetPeriodAsString(tempStr));
	lstAlgResult.SetItemText(result,count++,temp);
	
	lstAlgResult.RedrawItems(0,result);
	//
	
	
	return true;

//	|Se:|Sp|Acc:%3.2lf|%s\n",(res->Iteration+1),(int)res->tp,(int)res->tn,(int)res->fn,(int)res->fp,res->se,res->sp,res->acc,res->time.GetPeriodAsString(tempStr));
}

bool GraphicNotifier::InsertMessage(char* title,char* message)
{
	if ((title!=NULL) && (message!=NULL))
	{
		
		lstStatus.InsertItem(lstStatus.GetItemCount(), title);
		lstStatus.SetItemText(lstStatus.GetItemCount()-1,1,message); 
		return true;
	}
	return false;
}

bool GraphicNotifier::InsertError(char* text)
{
	if ((text!=NULL))
	{
		if(!InsertMessage("Error",text))
			return false;
		if(!lstStatus.SetItemData(lstStatus.GetItemCount()-1,CREATE_ITEM_STATUS(ITEM_TYPE_ERROR,0,0)))
			return false;
	}
	
	return false;
}

bool GraphicNotifier::InsertProgressControl(char* text)
{
	if(text==NULL)
		return false;

	if(!lstStatus.InsertItem(lstStatus.GetItemCount(), text))
		return false;
	
	if(!lstStatus.SetItemData(lstStatus.GetItemCount()-1,CREATE_ITEM_STATUS(ITEM_TYPE_STATUS,0,ITEM_CONTROLTYPE_PROGRESS_BAR)))
		return false;

	lstStatus.RedrawItems(lstStatus.GetItemCount()-1,lstStatus.GetItemCount()-1);
	lastProgressBar = lstStatus.GetItemCount()-1;
	return true;
}


bool GraphicNotifier::AddObject(CWnd* object, unsigned int tabId)
{
	
	if (tabId < 0 || tabId >= tabControl.GetItemCount())
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
	windowDestroyed = true;
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

	this->UpdateWindow();

}