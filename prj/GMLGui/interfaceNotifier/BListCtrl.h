#pragma once
#include "afxcmn.h"
#include "gmllib.h"



#define TYPE_PROGRESS_BAR 1
#define IDC_PROGRESS_LIST 500


struct ControlItem
{
	unsigned int line;
	unsigned int column;
	unsigned char type;
	unsigned int data;
	CWnd* wnd;
};


class BListCtrl : public CListCtrl
{
public:
	DECLARE_DYNAMIC(BListCtrl)
	GML::Utils::Vector m_controlItems;
	CFont simpleFont;

	BListCtrl(void);
	virtual ~BListCtrl(void);
	
	
	
	bool AddControlToItem(int i,int j,unsigned char controlType, unsigned int data = 0);
	ControlItem* GetControlItem(unsigned int i, unsigned int j);
	bool GetControlItemData(unsigned int line, unsigned int col, unsigned int* data);
	bool SetControlItemData(unsigned int line, unsigned int col, unsigned int data);
	
	
	bool HasControlItem(unsigned int i);
	int GetColumnsCount();
	

public:
	afx_msg void OnPaint();
	//afx_msg void OnDrawItem ( NMHDR* pNMHDR,LRESULT* pResult );
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	DECLARE_MESSAGE_MAP()
	
};

