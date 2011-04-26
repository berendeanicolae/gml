#pragma once
#include "afxwin.h"
#include "gml\gmllib.h"


#define TAB_CONTROL	0x1000
#define ID_RESULT_LIST 0x1001
#define ID_RESULT_EDIT 0x1002
#define MAX_CONTROLS 256
#define NR_TABS	3


#define TAB_LEFT 20
#define TAB_RIGHT 470
#define TAB_TOP 30
#define TAB_BOTTOM 460
#define TAB_WIDTH (TAB_RIGHT - TAB_LEFT)
#define TAB_HEIGHT (TAB_BOTTOM - TAB_TOP)

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
	CEdit			edtAlgOutput;
	
	CTabCtrl		tabControl;
	CStatic			temp;
	CListCtrl		lstAlgResult;
	tabOwner		tabControlObjects[MAX_CONTROLS];
	unsigned int	nrObjects;
	

public:
	bool			windowDestroyed;
	
	void Create(TCHAR* text,int x,int y,int width,int height,HWND parent);
	void Create(TCHAR* text,CRect& rect,HWND parent);

	bool AddObject(CWnd* object, unsigned int tabId);
	void OnTcnSelchange(NMHDR *pNMHDR, LRESULT *pResult);
	bool InsertStatistics(GML::Utils::AlgorithmResult	*res );
	bool InsertMessage(char* message, int DataSize,int MessageID);



	GraphicNotifier(void);
	~GraphicNotifier(void);


	afx_msg void OnClose();

	DECLARE_MESSAGE_MAP()
};
