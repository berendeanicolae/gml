#include "StdAfx.h"
#include "GraphicNotifier.h"


GraphicNotifier::GraphicNotifier(void)
{
	nrObjects = 0;
	windowDestroyed = false;
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
	tcItem.pszText = _T("Statistics");
	tabControl.InsertItem(0, &tcItem);
	tcItem.pszText = _T("Temp View");
	tabControl.InsertItem(1, &tcItem);
	tcItem.pszText = _T("Graphic View");
	tabControl.InsertItem(2, &tcItem);

	temp.Create(text,WS_VISIBLE,CRect(30,30,300,60),this);
	AddObject(&temp,1);



	lstAlgResult.CreateEx(LVS_EX_GRIDLINES|WS_EX_STATICEDGE,WS_BORDER| LVS_REPORT  | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_VISIBLE,CRect(TAB_LEFT,TAB_TOP,TAB_RIGHT,TAB_BOTTOM-100),this,ID_RESULT_LIST);
	

	
	lstAlgResult.SetFont(&fnt);

	//	printf("%4d|TP:%5d |TN:%5d |FN:%5d |FP:%5d |Se:%3.2lf|Sp:%3.2lf|Acc:%3.2lf|%s\n",(res->Iteration+1),(int)res->tp,(int)res->tn,(int)res->fn,(int)res->fp,res->se,res->sp,res->acc,res->time.GetPeriodAsString(tempStr));
	int count = 0;
	int nrColomns=9;


	lstAlgResult.InsertColumn(count++,"Nr",LVCFMT_CENTER,(TAB_WIDTH /nrColomns)/2);
	lstAlgResult.InsertColumn(count++,"TP",LVCFMT_CENTER,TAB_WIDTH /nrColomns);
	lstAlgResult.InsertColumn(count++,"TN",LVCFMT_CENTER,TAB_WIDTH /nrColomns);
	lstAlgResult.InsertColumn(count++,"FN",LVCFMT_CENTER,TAB_WIDTH /nrColomns);
	lstAlgResult.InsertColumn(count++,"FP",LVCFMT_CENTER,TAB_WIDTH /nrColomns);
	lstAlgResult.InsertColumn(count++,"Se",LVCFMT_CENTER,TAB_WIDTH /nrColomns);
	lstAlgResult.InsertColumn(count++,"Sp",LVCFMT_CENTER,TAB_WIDTH /nrColomns);
	lstAlgResult.InsertColumn(count++,"Acc",LVCFMT_CENTER,TAB_WIDTH /nrColomns);
	lstAlgResult.InsertColumn(count++,"Time",LVCFMT_LEFT,(TAB_WIDTH /nrColomns)*1.5);

	AddObject(&lstAlgResult,0);
	


	edtAlgOutput.Create(WS_VISIBLE| SS_SUNKEN | WS_BORDER| ES_AUTOVSCROLL |ES_AUTOHSCROLL | ES_LEFT|ES_MULTILINE |ES_NOHIDESEL  |ES_READONLY  |ES_WANTRETURN , CRect(TAB_LEFT,TAB_BOTTOM-90, TAB_RIGHT,TAB_BOTTOM),this,ID_RESULT_EDIT);
	
	edtAlgOutput.SetFont(&fnt);
	AddObject(&edtAlgOutput,0);
	//fortez redesenarea
	OnTcnSelchange(NULL,NULL);

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
END_MESSAGE_MAP()


GraphicNotifier::~GraphicNotifier(void)
{
}


bool GraphicNotifier::InsertStatistics(GML::Utils::AlgorithmResult	*res )
{
	GML::Utils::GString		tempStr;
	char temp[100];
	int result;
	int count = 0;

	
	sprintf(temp,"%d",res->Iteration+1);
	
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

bool GraphicNotifier::InsertMessage(char* text, int DataSize, int MessageID)
{
	CString temp,outStr,windowStr;

	outStr = "";

	if ((text!=NULL) && (DataSize>0))
	{
		for (UInt32 tr =0;tr<DataSize;tr++)
		{
			if ((text[tr]>=' ') && (text[tr]<=128))
				outStr+=text[tr];
			else
			{
				temp.Format("{\\x%02X}",(unsigned char)text[tr]);
				outStr+=temp;

			}
				
		}
	}
	
	
	edtAlgOutput.GetWindowTextA(windowStr);
	windowStr+="\r\n";
	windowStr+=outStr;
	edtAlgOutput.SetWindowTextA(windowStr);

	return true;
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

}