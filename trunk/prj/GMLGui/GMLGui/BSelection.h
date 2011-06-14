#pragma once
#include "BCombo.h"
#define COMBO_ELEMENT 0x4001
#define BUTTON_ELEMENT 0x4002


class BSelection :public BCombo
{
	

	CButton		button;
	UINT		buttonWidth;
public:
	BSelection(void);
	~BSelection(void);
	void*	btnClickArg;
	standardCallBack btnClickCallBack;
	bool	Create(char* strLabel,char* btnLabel, unsigned char offset,CWnd* parent, UINT txtID);
	void	UpdatePosition();

	bool SetCallbackBtnClick(standardCallBack f, void* arg);

	afx_msg void OnBtnClick();
	DECLARE_MESSAGE_MAP()
};


