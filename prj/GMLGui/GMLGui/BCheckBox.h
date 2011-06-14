#pragma once
#include "bitem.h"


class BCheckBox :
	public BItem
{
public:
	BCheckBox(void);
	~BCheckBox(void);
	CButton checkBox;

	void UpdatePosition();
	bool IsChecked();
	void SetCheck(bool value);
	bool Create(char* strLabel,unsigned char offset,CWnd* parent, UINT ID);



};

