#include "StdAfx.h"
#include "BListCtrl.h"

IMPLEMENT_DYNAMIC(BListCtrl,CListCtrl)
BListCtrl::BListCtrl()
{
	m_controlItems.Create(32, sizeof(ControlItem));
	simpleFont.CreateFontA(12,0,0,0,FW_REGULAR,FALSE,FALSE,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,TEXT("Arial"));
}


BListCtrl::~BListCtrl(void)
{
}


BEGIN_MESSAGE_MAP(BListCtrl, CListCtrl)
	ON_WM_PAINT()
	ON_WM_DRAWITEM()
END_MESSAGE_MAP()


ControlItem* BListCtrl::GetControlItem(unsigned int line, unsigned int col)
{
	for(int i = 0;i< m_controlItems.GetSize();i++)
	{
		ControlItem* item = (ControlItem*)m_controlItems.Get(i);
		if(item->line == line && item->column == col)
			return item;
	}
	return NULL;
}

bool BListCtrl::GetControlItemData(unsigned int line, unsigned int col, unsigned int* data)
{
	ControlItem* listItem = (ControlItem*)GetControlItem(line,col);
	if(listItem == NULL)
		return false;
	*data  = listItem->data;
	return true;
}

bool BListCtrl::SetControlItemData(unsigned int line, unsigned int col, unsigned int data)
{
	ControlItem* listItem = (ControlItem*)GetControlItem(line,col);
	if(listItem == NULL)
		return false;
	listItem->data = data;
	this->RedrawItems(line,line);
	return true;
}

bool BListCtrl::HasControlItem(unsigned int line)
{
	for(int i = 0;i< m_controlItems.GetSize();i++)
	{
		ControlItem* item = (ControlItem*)m_controlItems.Get(i);
		if(item->line == line)
			return true;
	}
	return false;
}


int BListCtrl::GetColumnsCount()  {return GetHeaderCtrl()->GetItemCount();}

bool BListCtrl::AddControlToItem(int i,int j,unsigned char controlType, unsigned int data)
{
	ControlItem item;

	if( i< 0 || i>= GetItemCount())
		return false;
	item.line = i;
	item.column =j;
	item.type = controlType;
	item.wnd = NULL;
	item.data = data;
	m_controlItems.Push(&item);
	return true;

}
void BListCtrl::OnPaint()
{
	// TODO: Add your message handler code here
	// Do not call CListCtrl::OnPaint() for painting messages

	int percent;
	int Top=GetTopIndex();
	int Total=GetItemCount();
	int PerPage=GetCountPerPage();
	int LastItem=((Top+PerPage)>Total)?Total:Top+PerPage;

	// if the count in the list os not zero delete all the progress controls and them procede
	{
		int Count=(int)m_controlItems.GetSize();
		for(int i=0;i<Count;i++)
		{
			ControlItem* controlItem=(ControlItem*)m_controlItems.Get(i);
			if(IsWindow(controlItem->wnd->GetSafeHwnd()))
				controlItem->wnd->DestroyWindow();
			//m_controlItems.RemoveAt(0);
		}
	}

	
	CHeaderCtrl* pHeader=GetHeaderCtrl();

	

	for(int i=Top;i<LastItem;i++)
	{
		CRect ColRt;
		if(HasControlItem(i))
		{
			for(int j=0;j<GetColumnsCount();j++)
			{
				ControlItem* item = GetControlItem(i,j);
				if(item != NULL)
				{
					pHeader->GetItemRect(j,&ColRt);	
					// get the rect
					CRect rt;
					GetItemRect(i,&rt,LVIR_LABEL);
					rt.top+=1;
					rt.bottom-=1;
					rt.left+=ColRt.left+2;
					int Width=ColRt.Width();
					rt.right=rt.left+Width-8;
					/*
					rt.left=ColRt.left+1;
					rt.right=ColRt.right-1;
					*/

					// create the progress control and set their position
					if(item->type == TYPE_PROGRESS_BAR)
					{
							CProgressCtrl* pControl=new CProgressCtrl();
							pControl->Create(WS_VISIBLE | WS_CHILD,rt,this,IDC_PROGRESS_LIST+i);
							percent = item->data;
							// set the position on the control
							pControl->SetPos(percent);
							pControl->ShowWindow(SW_SHOWNORMAL);
							item->wnd = pControl;
					}
				
				}
			}
		}
		
		

		
	}
	CListCtrl::OnPaint();
}


void BListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	COLORREF crText;
	CRect rItem;
	CDC* dc;
	CString text;
	CRect ColRt;


	rItem = lpDrawItemStruct->rcItem;
	dc = CDC::FromHandle(lpDrawItemStruct->hDC);
	//GetText(lpDrawItemStruct->itemID, text);
	


	
	// If CListBox item selected, draw the highlight rectangle.
	// Or if CListBox item deselected, draw the rectangle using the window color.
	if ((lpDrawItemStruct->itemState & ODS_SELECTED) && (lpDrawItemStruct->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
	{
		CBrush br(::GetSysColor(COLOR_HIGHLIGHT));
		dc->FillRect(&rItem, &br);
	}
	else if (!(lpDrawItemStruct->itemState & ODS_SELECTED) && (lpDrawItemStruct->itemAction & ODA_SELECT)) 
	{
		CBrush br(::GetSysColor(COLOR_WINDOW));
		dc->FillRect(&rItem, &br);
	}

	// If the CListBox item has focus, draw the focus rect.
	// If the item does not have focus, erase the focus rect.
	if ((lpDrawItemStruct->itemAction & ODA_FOCUS) && (lpDrawItemStruct->itemState & ODS_FOCUS))
	{
		dc->DrawFocusRect(&rItem); 
	}
	else if ((lpDrawItemStruct->itemAction & ODA_FOCUS) && !(lpDrawItemStruct->itemState & ODS_FOCUS))
	{
		dc->DrawFocusRect(&rItem); 
	}

	

	int iBkMode = dc->SetBkMode(TRANSPARENT);
	if(GetItemText(lpDrawItemStruct->itemID,0).Compare("Error")==0)
	{
		crText = RGB(255,0,0);
	}
	else
		crText = COLOR_3DFACE;
	

	//Draw the Text
	CHeaderCtrl* pHeader=GetHeaderCtrl();

	for(int i=0;i<GetColumnsCount();i++)
	{
		pHeader->GetItemRect(i,&ColRt);		
		text = this->GetItemText(lpDrawItemStruct->itemID,i);
		if(text.GetLength() > 0)
		{
			ColRt.left+=5;
			ColRt.right-=5;
			ColRt.top = lpDrawItemStruct->rcItem.top;
			ColRt.bottom = lpDrawItemStruct->rcItem.bottom;
			dc->SetTextColor(crText);
			dc->SelectObject(&simpleFont);
			dc->DrawText(text,ColRt,DT_LEFT);
			//dc->TextOut(ColRt.left+5,rItem.top,text.GetString());	
		}
	}
	

}
/*
void BListCtrl::OnDrawItem(NMHDR* pNMHDR, LRESULT* pResult )
{
	CString text;
	LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)pNMHDR;

	int nRow = lplvcd->nmcd.dwItemSpec; // get row number
	text = GetItemText(nRow,0);
	if(text.CompareNoCase("[BitConnector] ")==0)
	{
		lplvcd->clrText = RGB(255,0,0);
	}

	*pResult=CDRF_NEWFONT|CDRF_NOTIFYSUBITEMDRAW;
}
*/