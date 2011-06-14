#include "StdAfx.h"
#include "BSelection.h"


BSelection::BSelection(void)
{
	selChangeCallBack= NULL;
	selChangeArg = NULL;
	btnClickArg = NULL;
	btnClickCallBack = NULL;
}


BSelection::~BSelection(void)
{
}


BEGIN_MESSAGE_MAP(BSelection, BCombo)
	ON_BN_CLICKED(BUTTON_ELEMENT,&BSelection::OnBtnClick)
END_MESSAGE_MAP()



void BSelection::OnBtnClick()
{
	if(btnClickCallBack != NULL)
		btnClickCallBack(btnClickArg);
}

bool BSelection::Create(char* strLabel,char* btnLabel, unsigned char offset,CWnd* parent, UINT id)
{
	
	DWORD elementWidth;
	BItem::Create(strLabel,offset,ELEMENT_WIDTH,LABEL_WIDTH,parent);
	buttonWidth = strlen(btnLabel)*7;

	CRect tempRect;
	GetClientRect(&tempRect);
	tempRect.left = tempRect.left+this->LabelWidth-(offset*OFFSET_SIZE)+SPACE_BETWEEN_ELEMENT;
	tempRect.right = tempRect.left+ tempRect.Width() - buttonWidth - SPACE_BETWEEN_ELEMENT;
	tempRect.bottom+=100;
	if(!comboBox.Create(WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,  tempRect,this,COMBO_ELEMENT)){return false;};	

	//tempRect.right+=SPACE_BETWEEN_ELEMENT;
	
	comboBox.SetFont(&simpleFont,true);
	tempRect.bottom-=100;
	tempRect.left = tempRect.right + SPACE_BETWEEN_ELEMENT;
	tempRect.right = tempRect.left + buttonWidth;
	if(!button.Create(btnLabel,WS_CHILD | WS_VISIBLE |BS_PUSHBUTTON,tempRect,this,BUTTON_ELEMENT)){return false;}
	button.SetFont(&simpleFont,true);
	
	UpdatePosition();

	return true;
}

void BSelection::UpdatePosition()
{
	CRect tempRect;
	DWORD elementWidth;

	GetClientRect(&tempRect);
	tempRect.left = this->LabelWidth +  SPACE_BETWEEN_ELEMENT;
	tempRect.right = tempRect.left+ tempRect.Width() - buttonWidth- SPACE_BETWEEN_ELEMENT;
	comboBox.MoveWindow(tempRect);
	tempRect.left = tempRect.right + SPACE_BETWEEN_ELEMENT;
	tempRect.right = tempRect.left + buttonWidth;
	button.MoveWindow(tempRect);
	
}
bool BSelection::SetCallbackBtnClick(standardCallBack f, void* arg)
{
	if(f == NULL)
		return false;
	btnClickCallBack = f;
	this->btnClickArg = arg;
	return true;

}
