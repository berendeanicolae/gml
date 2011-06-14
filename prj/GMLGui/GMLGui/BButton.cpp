#include "StdAfx.h"
#include "BButton.h"


BButton::BButton(void)
{
}


BButton::~BButton(void)
{
}


BEGIN_MESSAGE_MAP(BButton,BItem)
	ON_BN_CLICKED(BUTTON_ELEMENT,&BButton::OnBnClicked)
END_MESSAGE_MAP()

bool BButton::Create(char* strLabel,char* buttonLabel,unsigned char offset,CWnd* parent, UINT ID)
{
		
	//tempRect.right -=SPACE_BETWEEN_ELEMENT;
	BItem::Create(strLabel,offset,ELEMENT_WIDTH,LABEL_WIDTH,parent);

	CRect tempRect;
	GetClientRect(&tempRect);
	tempRect.left = tempRect.left+this->LabelWidth;//-(offset*OFFSET_SIZE)+SPACE_BETWEEN_ELEMENT;
	buttonWidth = strlen(buttonLabel)*8;
	
	//tempRect.right+=SPACE_BETWEEN_ELEMENT;
	tempRect.left = (tempRect.Width() - buttonWidth) /2;
	tempRect.right = buttonWidth;
	
	if(!button.Create(buttonLabel,WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,  tempRect,this,BUTTON_ELEMENT)){return false;};	
	button.SetFont(&simpleFont,true);
	UpdatePosition();

	return true;
}

void BButton::UpdatePosition()
{
	CRect tempRect;

	GetClientRect(&tempRect);
	
	tempRect.left = tempRect.Width() - buttonWidth;
	tempRect.right = tempRect.left+buttonWidth;
	button.MoveWindow(tempRect);
}

bool BButton::SetCallbackBtnClick(standardCallBack f, void* arg)
{
	if(f == NULL)
		return false;
	btnCallBack = f;
	this->arg = arg;
	return true;
}

void BButton::OnBnClicked()
{
	if(btnCallBack != NULL)
		btnCallBack(arg);
}

bool BButton::GetString(CString* str)
{
	GetWindowText(*str);
	return true;
}


