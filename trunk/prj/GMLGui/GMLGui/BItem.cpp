#include "StdAfx.h"
#include "BItem.h"
#include "BContainer.h"


BItem::BItem(void)
{
	offset = 0;
	collapsed = 0;
	LabelWidth = LABEL_WIDTH;
	description = NULL;
	metadata = NULL;
	elementType = 0;
}


BItem::~BItem(void)
{
	if (description != NULL)
		delete description;
	description = NULL;

	if (metadata != NULL)
		delete metadata;
	metadata = NULL;

}

BEGIN_MESSAGE_MAP(BItem, CWnd)
	//{{AFX_MSG_MAP(EPropListCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_PAINT()
	
	//ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


DWORD BItem::GetHeight()
{
	CRect rect;
	GetClientRect(&rect);

	return rect.Height();
}

bool BItem::IsVisible()
{
	if(this->GetStyle() & WS_VISIBLE)
		return true;
	return false;
}

bool BItem::Create(char* strLabel,unsigned char offset,DWORD Width,DWORD LabelWidth,CWnd* parent)
//bool BItem::Create(char* strLabel,unsigned char offset, CRect* totalItemRect,CWnd* parent)
{
	CRect tempRect;

	tempRect.left = 0;
	tempRect.right = Width;
	tempRect.top = 0;
	tempRect.bottom = ELEMENT_HEIGHT;
	
	
	//DWORD tempRight = tempRect.right;
	

	if(!simpleFont.CreateFontA(14,0,0,0,FW_REGULAR,FALSE,FALSE,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,TEXT("Arial"))) return false;
	if(!smallFont.CreateFontA(10,0,0,0,FW_REGULAR,FALSE,FALSE,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,TEXT("Arial"))) return false;

	//tempRect.right = tempRect.left + LABEL_WIDTH - (offset *OFFSET_SIZE);
	//tempRect.top+=3;
	
	if(!CWnd::Create("STATIC",strLabel,WS_CHILD | WS_VISIBLE | SS_NOTIFY  ,tempRect,parent,0,0)) {return false;}
	strncpy(label,strLabel,100);
	SetFont(&simpleFont,true);

	this->SetFont(&simpleFont);
	this->offset = offset;
	description = NULL;
	metadata = NULL;

	return true;
}


DWORD BItem::MoveElement(DWORD newYPos)
{
	CRect rect;
	DWORD height;
	::GetWindowRect(this->m_hWnd,&rect);
	this->ScreenToClient(&rect);

	height = rect.Height();
	rect.top = newYPos;
	rect.bottom = newYPos+ height;
	this->MoveWindow(&rect);
	
	return rect.bottom;
}

void BItem::OnPaint()
{
	Paint();
	CWnd::OnPaint();
}


void BItem::OnClick(UINT nFlags,CPoint point)
{
	CRect collapseButton,cr;

	GetClientRect(&cr);
	cr.right = LabelWidth;

	if(collapsed == ITEM_NO_COLLAPSE)
		return;
	collapseButton.SetRect(cr.Width()-15,5,cr.Width()-5,15);
	if(collapseButton.PtInRect(point))
	{
		if(collapsed == ITEM_COLLAPSED)
			collapsed = ITEM_EXPANDED;
		else
			if(collapsed == ITEM_EXPANDED)
				collapsed = ITEM_COLLAPSED;
			
		InvalidateRect(collapseButton);
		BContainer* container = (BContainer*)this->GetParent();
		container->ShowChildren();
	}
}

afx_msg void BItem::OnLButtonDown(UINT nFlags,CPoint point )
{
	OnClick(nFlags,point);

}

void BItem::SetCollapsed(char state)
{
	collapsed = state;
}

char BItem::GetCollapsedState()
{
	return collapsed;
}


bool BItem::SetMetaData(char* md)
{
	if(metadata != NULL)
	{
		delete metadata;
		metadata = NULL;
	}
	if(md == NULL)
		return true;

	unsigned int length = strlen(md);
	metadata = new char[length+1];
	if(metadata == NULL)
		return false;
	memcpy(metadata,md,length);
	metadata[length] = '\0';
	return true;
}

char* BItem::GetMetaData()
{
	return metadata;
}
bool BItem::SetDescription(char* des)
{
	if(description != NULL)
	{
		delete description;
		description = NULL;
	}
	if(des == NULL)
		return true;
	unsigned int length = strlen(des);
	if(description != NULL)
		delete description;
	description = new char[length+1];
	if(description == NULL)
		return false;
	memcpy(description,des,length);
	description[length] = '\0';

	return true;

}


void BItem::UpdatePosition()
{
	return;
}

void BItem::Paint()
{
	CRect   cr;

	
	  

	GetClientRect(&cr);
	CPaintDC dc(this);
	CDC* pDC = &dc;
	CDC* pParsedDC = pDC;
	CDC     MemDC;
	CBitmap MemBM;


	MemBM.CreateCompatibleBitmap(pParsedDC,cr.Width(),cr.Height());
	MemDC.CreateCompatibleDC(pParsedDC);
	MemDC.SelectObject(&MemBM);
	
	pDC = &MemDC;
	
	//umple backgroundul

	CRect fillRect;
    fillRect.SetRect(0,0,cr.Width(),cr.Height());
    pDC->FillSolidRect(&fillRect, RGB(240,240,240));
	
	
	CRect largerRect;
	//tempRect.right = tempRect.left + LABEL_WIDTH - (offset *OFFSET_SIZE);
	//tempRect.top+=3;
	
	//tempRect.right = tempRect.left + LABEL_WIDTH - (offset *OFFSET_SIZE);
	//tempRect.top+=3;
	//largerRect.SetRect(0,0,cr.Width() /2,cr.Height());
	cr.left +=(offset *OFFSET_SIZE);
	cr.right = LabelWidth;
	
	
	pDC->DrawEdge(&cr,BDR_RAISEDINNER,BF_RECT);
	
	//pDC->SetBkMode(TRANSPARENT);
//	pDC->SetBkColor(RGB(240,240,240));
	
	pDC->SelectObject(&simpleFont);
	CRect textRext;
	textRext.SetRect(cr.left+5,cr.Height()/2 - 7,cr.right-20,cr.Height());
	
	pDC->DrawText(label,-1,textRext,DT_LEFT);

	

	//deseneaza buotnul de plus
	if(collapsed)
	{
		
		CRect collapseButton;
		collapseButton.SetRect(cr.right-15,5,cr.right-4,16);
		pDC->DrawEdge(&collapseButton,BDR_RAISEDINNER,BF_RECT);
	
		collapseButton.SetRect(cr.right-13,4,cr.right-7,15);
		pDC->SelectObject(&smallFont);
		pDC->MoveTo(cr.right-12,10);
		pDC->LineTo(cr.right-7,10);

		if(collapsed == ITEM_COLLAPSED)
		{
			pDC->MoveTo(cr.right-10,8);
			pDC->LineTo(cr.right-10,13);
		}
	}
	

	
	
	
//--------------------------
// flicker
//--------------------------
	//pDC->SetViewportOrg(0,0);
	//MemDC.SetViewportOrg(0,0);
	pParsedDC->BitBlt( 0 , 0 , LabelWidth , cr.Height() , &MemDC , 0 , 0 , SRCCOPY );
	
}

