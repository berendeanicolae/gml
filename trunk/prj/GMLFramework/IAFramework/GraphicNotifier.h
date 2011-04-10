#pragma once
#include "afxwin.h"


#define TAB_CONTROL	0x1000
#define ID_RESULT_LIST 0x1001
#define MAX_CONTROLS 256
#define NR_TABS	3


#define TAB_LEFT 20
#define TAB_RIGHT 470
#define TAB_TOP 30
#define TAB_BOTTOM 460

typedef struct  
{
		CWnd* object;
		unsigned char tabId;
}tabOwner;

class GraphicNotifier :
	public CWnd
{

	int width,height,x,y;
	CBrush			someBrush;
	CFont			fnt;
	CEdit			testEdit1;
	
	CTabCtrl		tabControl;
	CStatic			temp;
	CListCtrl		lstAlgResult;
	tabOwner		tabControlObjects[MAX_CONTROLS];
	unsigned int	nrObjects;

public:
	void Create(TCHAR* text,int x,int y,int width,int height,CWnd *parent);
	void Create(TCHAR* text,CRect& rect,CWnd *parent);

	bool AddObject(CWnd* object, unsigned int tabId);
	void OnTcnSelchange(NMHDR *pNMHDR, LRESULT *pResult);

	GraphicNotifier(void);
	~GraphicNotifier(void);


	afx_msg void OnPaint();
	afx_msg void OnClose();

	DECLARE_MESSAGE_MAP()
};

