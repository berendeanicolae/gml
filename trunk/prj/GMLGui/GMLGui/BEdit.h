#pragma once
#include "afxwin.h"
#include "BItem.h"


class BEdit : public BItem
{
public:
	BEdit(void);
	~BEdit(void);

	
	CEdit	textBox;
	char*	text;

	void	SetText(char* t);
	void	GetText(CString& str);
	bool	Create(char* strLabel,unsigned char offset,CWnd* parent, UINT txtID);
	void	UpdatePosition();
};

