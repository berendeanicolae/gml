
// IAFrameworkDlg.h : header file
//

#pragma once

#include "EPropCtrl.h"
#include "AlgorithmObject.h"
#include "afxwin.h"
#include "GraphicNotifierContainer.h"



#define WM_ALGORITHM_CONTAINER_CLOSED (WM_USER+1)
#define ID_ALGORITHM_CONTAINER 0x1000


// CIAFrameworkDlg dialog
class CIAFrameworkDlg : public CDialogEx
{
// Construction

	EPropCtrl m_PropCtrl;
public:
	CIAFrameworkDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_IAFRAMEWORK_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	GraphicNotifierContainer* graphicResultWindow;


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	bool FindAlgorithms();

	DECLARE_MESSAGE_MAP()

	AlgorithmObject* currentAlgorithm;
public:
	CComboBox cmbAlgorithms;
	afx_msg void OnCbnSelchangeCmbAlgorithms();
	afx_msg void OnFileSaveconfiguration();
	afx_msg void OnFileOpenconfiguration();
	LRESULT OnAlgorithmContainerClosed(WPARAM p1, LPARAM p2);

	void DrawSelectedAlgorithm(char* configurationFile=NULL);
	bool AlgorithmRun(GML::Utils::AttributeList*	attrList);
	bool StartGmlExec(GML::Utils::AttributeList*	attrList);
	afx_msg void OnFileRun();

	afx_msg void OnOK();
};
