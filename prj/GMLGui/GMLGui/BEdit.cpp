#include "StdAfx.h"
#include "BEdit.h"


BEdit::BEdit(void)
{
}


BEdit::~BEdit(void)
{
}

bool BEdit::Create(char* strLabel,unsigned char offset,CWnd* parent, UINT txtID)
{
	;
	//tempRect.right -=SPACE_BETWEEN_ELEMENT;
	BItem::Create(strLabel,offset,ELEMENT_WIDTH,LABEL_WIDTH,parent);

	CRect tempRect;
	GetClientRect(&tempRect);
	tempRect.left = tempRect.left+this->LabelWidth-(offset*OFFSET_SIZE)+SPACE_BETWEEN_ELEMENT;
	
	
	//tempRect.right+=SPACE_BETWEEN_ELEMENT;
	if(!textBox.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL, tempRect,this,txtID)){return false;};
	textBox.SetFont(&simpleFont,true);
	UpdatePosition();

	return true;
}

void BEdit::UpdatePosition()
{
	CRect tempRect;

	GetClientRect(&tempRect);
	tempRect.left = this->LabelWidth +  SPACE_BETWEEN_ELEMENT;
	textBox.MoveWindow(tempRect);
}

void BEdit::SetText(char* t)
{
	textBox.SetWindowText(t);
}

void BEdit::GetText(CString& str)
{
	this->textBox.GetWindowText(str);
}
