#pragma once
#include "afxcmn.h"
#include "GraphicNotifier.h"


#define MAX_ALGORITHMS 100
#define ALGORITHM_TAB_CONTROL 500
#define WM_ALGORITHM_CONTAINER_CLOSED (WM_USER+1)
#define WM_NEW_ALGORITHM (WM_USER+2)
#define WM_DELETE_ALGORITHM (WM_USER+3)

class GraphicNotifierContainer :public CWnd
{
	GraphicNotifier* algorithmObjects[MAX_ALGORITHMS];
	unsigned int nrAlgorithms;
	CWnd* parentWindow;

	unsigned int x,y, width, height;
	CBrush someBrush;
	CFont fnt;
	CTabCtrl currentTabControl;

	//temp
	HMODULE hModule;
	

public:
	GraphicNotifierContainer(void);
	~GraphicNotifierContainer(void);

	void Create(TCHAR* text,int x,int y,int width,int height,CWnd *parent, UINT ID=NULL);
	bool AddAlgorithm(char* algorithmName);
	void OnTcnSelchange(NMHDR *pNMHDR, LRESULT *pResult);
	LRESULT OnNewAlgorithm(WPARAM p1, LPARAM p2);
	LRESULT OnCloseAlgorithm(WPARAM p1, LPARAM p2);
	afx_msg void OnClose();

	DECLARE_MESSAGE_MAP()

	
};

