#include "StdAfx.h"
#include "BCombo.h"


BCombo::BCombo(void)
{
selChangeCallBack= NULL;
selChangeArg = NULL;
}


BCombo::~BCombo(void)
{
}

BEGIN_MESSAGE_MAP(BCombo, BItem)
	ON_CBN_SELCHANGE(COMBO_ELEMENT, &BCombo::OnCbnSelchange)
END_MESSAGE_MAP()


void BCombo::OnCbnSelchange()
{
	if(selChangeCallBack != NULL)
		selChangeCallBack(selChangeArg);
}

bool BCombo::SetCallbackCbnSelChange(standardCallBack f, void* arg)
{
	if(f == NULL)
		return false;
	selChangeCallBack = f;
	this->selChangeArg = arg;
	return true;
}

bool BCombo::SetSelectedItem(unsigned int i)
{
	if(i >= this->GetNrItems())
		return false;
	comboBox.SetCurSel(i);
	return true;
}
bool BCombo::GetSelectedItem(CString &el)
{
	int selElement = comboBox.GetCurSel();
	if(selElement <0)
		return false;
	return this->GetItem(selElement,el);
}
int BCombo::GetSelectedElement()
{
	return comboBox.GetCurSel();
}

bool BCombo::Create(char* strLabel,unsigned char offset,CWnd* parent, UINT ID)
{
		
	//tempRect.right -=SPACE_BETWEEN_ELEMENT;
	BItem::Create(strLabel,offset,ELEMENT_WIDTH,LABEL_WIDTH,parent);

	CRect tempRect;
	GetClientRect(&tempRect);
	tempRect.left = tempRect.left+this->LabelWidth-(offset*OFFSET_SIZE)+SPACE_BETWEEN_ELEMENT;
	tempRect.bottom+=100;
	
	//tempRect.right+=SPACE_BETWEEN_ELEMENT;

	
	if(!comboBox.Create(WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,  tempRect,this,COMBO_ELEMENT)){return false;};	
	comboBox.SetFont(&simpleFont,true);
	UpdatePosition();

	return true;
}

bool BCombo::AddString(char* str)
{
	if(str == NULL)
		return false;
	if(!comboBox.AddString(str))
		return false;
	return true;
}

UINT BCombo::GetNrItems()
{
	return comboBox.GetCount();
}

bool BCombo::GetItem(unsigned int index,CString& el)
{
	if(index >= comboBox.GetCount())
		return false;
	comboBox.GetLBText(index,el);
	return true;
}

void BCombo::UpdatePosition()
{
	CRect tempRect;

	GetClientRect(&tempRect);
	tempRect.left = this->LabelWidth +  SPACE_BETWEEN_ELEMENT;
	tempRect.right+=2;
	comboBox.MoveWindow(tempRect);
}


