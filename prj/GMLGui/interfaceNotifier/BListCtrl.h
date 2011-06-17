#pragma once
#include "afxcmn.h"


#define GET_ITEM_TYPE(x) ((x & 0x000000ff))
#define GET_ITEM_STATUS(x) ((x & 0x0000ff00) >> 8)
#define GET_ITEM_CONTROLTYPE(x) ((x & 0x00ff0000) >> 16)
#define CREATE_ITEM_STATUS(type,status,controlType) ((type % 255) |  ((status % 255)<< 8)| ((controlType % 255)<< 16))

#define ITEM_CONTROLTYPE_PROGRESS_BAR 1

#define ITEM_TYPE_STATUS 1
#define ITEM_TYPE_ERROR	2

class BListCtrl : public CListCtrl
{
public:
	DECLARE_DYNAMIC(BListCtrl)
	CFont simpleFont;

	BListCtrl(void);
	virtual ~BListCtrl(void);	
	int GetColumnsCount();
	bool SetProgress(int itemId, int progress);
	

public:
	//afx_msg void OnPaint();
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	DECLARE_MESSAGE_MAP()
	
};


