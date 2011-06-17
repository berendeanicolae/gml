#include "StdAfx.h"
#include "BListCtrl.h"

IMPLEMENT_DYNAMIC(BListCtrl,CListCtrl)
BListCtrl::BListCtrl()
{
	simpleFont.CreateFontA(12,0,0,0,FW_REGULAR,FALSE,FALSE,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,TEXT("Arial"));
}


BListCtrl::~BListCtrl(void)
{
}


BEGIN_MESSAGE_MAP(BListCtrl, CListCtrl)
	//ON_WM_PAINT()
	ON_WM_DRAWITEM()
END_MESSAGE_MAP()




int BListCtrl::GetColumnsCount()  {return GetHeaderCtrl()->GetItemCount();}




void BListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	COLORREF crText;
	CRect rItem;
	CDC* dc;
	CString text;
	CRect ColRt;
	DWORD itemData;
	unsigned char controlType;
	unsigned char itemStatus;


	rItem = lpDrawItemStruct->rcItem;
	dc = CDC::FromHandle(lpDrawItemStruct->hDC);
		
	if(GetItemState(lpDrawItemStruct->itemID, LVIS_SELECTED)& LVIS_SELECTED)
	{
		CBrush br(::GetSysColor(COLOR_HIGHLIGHT));
		dc->FillRect(&rItem, &br);

	}
	else
	{
		CBrush br(::GetSysColor(COLOR_WINDOW));
		dc->FillRect(&rItem, &br);
	}

	int iBkMode = dc->SetBkMode(TRANSPARENT);	
	//Draw the Text
	CHeaderCtrl* pHeader=GetHeaderCtrl();

	itemData = GetItemData(lpDrawItemStruct->itemID);
	crText = RGB(0,0,0);
	if(itemData != 0)
	{
		if(GET_ITEM_TYPE(itemData) == ITEM_TYPE_ERROR)
			crText = RGB(255,0,0);
	}

	for(int i=0;i<GetColumnsCount();i++)
	{
		pHeader->GetItemRect(i,&ColRt);			
		ColRt.left+=rItem.left;
		ColRt.left+=5;
		ColRt.right+=rItem.left;
		ColRt.right-=5;
		ColRt.top = lpDrawItemStruct->rcItem.top;
		ColRt.bottom = lpDrawItemStruct->rcItem.bottom;
		if( i == 0 || GET_ITEM_CONTROLTYPE(itemData) == 0)
		{
			text = this->GetItemText(lpDrawItemStruct->itemID,i);	
			if(text.GetLength() > 0)
			{
				
				dc->SetTextColor(crText);
				dc->SelectObject(&simpleFont);
				dc->DrawText(text,ColRt,DT_LEFT);
				
			}
		}
		else
		{
			controlType = GET_ITEM_CONTROLTYPE(itemData);
			itemStatus = GET_ITEM_STATUS(itemData);
			if(controlType == ITEM_CONTROLTYPE_PROGRESS_BAR)
			{
				double length;
				
				dc->DrawFocusRect(ColRt);
				ColRt.top+=1;
				ColRt.bottom-=1;
				length = ColRt.Width() * ((double)itemStatus/100);
				ColRt.right = ColRt.left + length;
				CBrush br(::GetSysColor(COLOR_HIGHLIGHT));
				dc->FillRect(ColRt,&br);
			}
		}
		
		
		
	}
	

}

bool BListCtrl::SetProgress(int itemId, int progress)
{
	DWORD itemData;
	if(itemId<0 || itemId>=GetItemCount())
		return false;
	if(progress <0 || progress > 100)
		return false;
	itemData = GetItemData(itemId);
	if(itemData == 0)
		return false;
	if(GET_ITEM_CONTROLTYPE(itemData)!= ITEM_CONTROLTYPE_PROGRESS_BAR)
		return false;
	return SetItemData(itemId,CREATE_ITEM_STATUS(GET_ITEM_TYPE(itemData),progress,GET_ITEM_CONTROLTYPE(itemData)));
}