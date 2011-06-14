#pragma once
#include "bitem.h"

#define COMBO_ELEMENT 0x4000
typedef void (*standardCallBack)(void*);

class BCombo :
	public BItem
{
public:
	BCombo(void);
	~BCombo(void);
	CComboBox comboBox;
	standardCallBack selChangeCallBack;

	void*	selChangeArg;

	bool Create(char* strLabel,unsigned char offset,CWnd* parent, UINT ID);
	bool AddString(char* str);
	UINT GetNrItems();
	bool GetItem(unsigned int index,CString& el);
	void UpdatePosition();
	int GetSelectedElement();
	bool SetCallbackCbnSelChange(standardCallBack f, void* arg);
	
	bool GetSelectedItem(CString &el);
	bool SetSelectedItem(unsigned int i);

	
	afx_msg void OnCbnSelchange();
	DECLARE_MESSAGE_MAP()
};

