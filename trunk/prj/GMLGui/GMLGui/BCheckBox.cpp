#include "StdAfx.h"
#include "BCheckBox.h"


BCheckBox::BCheckBox(void)
{
}


BCheckBox::~BCheckBox(void)
{
}



bool BCheckBox::Create(char* strLabel,unsigned char offset,CWnd* parent, UINT ID)
{
		
	//tempRect.right -=SPACE_BETWEEN_ELEMENT;
	BItem::Create(strLabel,offset,ELEMENT_WIDTH,LABEL_WIDTH,parent);

	CRect tempRect;
	GetClientRect(&tempRect);
	tempRect.left = tempRect.left+this->LabelWidth;//-(offset*OFFSET_SIZE)+SPACE_BETWEEN_ELEMENT;
	
	//tempRect.right+=SPACE_BETWEEN_ELEMENT;

	
	if(!checkBox.Create(strLabel,WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,  tempRect,this,ID)){return false;};	
	checkBox.SetFont(&simpleFont,true);
	UpdatePosition();

	return true;
}

void BCheckBox::UpdatePosition()
{
	CRect tempRect;

	GetClientRect(&tempRect);
	tempRect.left = this->LabelWidth +  SPACE_BETWEEN_ELEMENT+10;
	checkBox.MoveWindow(tempRect);
}

bool BCheckBox::IsChecked()
{
	return (checkBox.GetCheck() == BST_CHECKED);
}

void BCheckBox::SetCheck(bool value)
{
	if(value == true)
		checkBox.SetCheck(BST_CHECKED);
	else
		checkBox.SetCheck(BST_UNCHECKED);

}