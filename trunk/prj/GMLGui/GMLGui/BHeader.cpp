#include "StdAfx.h"
#include "BHeader.h"
#include "BContainer.h"


BHeader::BHeader(void)
{
}


BHeader::~BHeader(void)
{
}



void BHeader::Paint()
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
    pDC->FillSolidRect(&fillRect, RGB(220,220,220));
	

//	cr.left = 0;
//	cr.right = LabelWidth;
//	pDC->FillSolidRect(&cr,RGB(255,0,0));
	
	pDC->DrawEdge(&cr,BDR_RAISEDINNER,BF_RECT);
	
	//pDC->SetBkMode(TRANSPARENT);
//	pDC->SetBkColor(RGB(240,240,240));
	
	pDC->SelectObject(&simpleFont);
	CRect textRext;
	textRext.SetRect(cr.left+5,cr.Height()/2 - 7,cr.right-20,cr.Height());
	
	pDC->SetTextColor(RGB(0,0,0));
	pDC->DrawText(label,-1,textRext,DT_LEFT);


//	fillRect.left =cr.right+SPACE_BETWEEN_ELEMENT;
//	fillRect.right = 500;
//	pDC->FillSolidRect(&fillRect,RGB(255,0,0));
	
	//deseneaza buotnul de plus
	//CRect plusButton;
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
	pParsedDC->BitBlt( 0 , 0 , cr.Width() , cr.Height() , &MemDC , 0 , 0 , SRCCOPY );
	
}


bool BHeader::Create(char* strLabel,unsigned char offset,CWnd* parent, UINT txtID)
{
	
	//tempRect.right -=SPACE_BETWEEN_ELEMENT;
	BItem::Create(strLabel,offset,ELEMENT_WIDTH,LABEL_WIDTH,parent);

	CRect tempRect;
	GetClientRect(&tempRect);


	return true;
}


void BHeader::OnClick(UINT nFlags,CPoint point)
{
	CRect collapseButton,cr;

	GetClientRect(&cr);
	//cr.right = LabelWidth;

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