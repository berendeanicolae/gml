#pragma once
#include "bitem.h"


#define BUTTON_ELEMENT 0x4003
typedef void (*standardCallBack)(void*);

class BButton :
	public BItem
{
public:
	BButton(void);
	~BButton(void);
	CButton button;
	UINT buttonWidth;

	standardCallBack btnCallBack;
	void*	arg;

	void UpdatePosition();
	bool Create(char* strLabel,char* buttonLabel,unsigned char offset,CWnd* parent, UINT ID);
	bool SetCallbackBtnClick(standardCallBack f, void* arg);
	bool GetString(CString* str);

	
	afx_msg void OnBnClicked();
	DECLARE_MESSAGE_MAP()
};

