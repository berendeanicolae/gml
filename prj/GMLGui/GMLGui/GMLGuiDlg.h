
// GMLGuiDlg.h : header file
//

#pragma once

#include "BContainer.h"
#include "afxwin.h"
#include "gmllib.h"
#include "GraphicNotifierContainer.h"

#define MAX_CONNECTORS	50
#define WM_ALGORITHM_CONTAINER_CLOSED (WM_USER+1)
#define ID_ALGORITHM_CONTAINER 0x1000

// CGMLGuiDlg dialog

struct AlgorithmRunConfig
{
	CString algorithmName;
	GML::Utils::AttributeList* attrList;
	void* container;
};

class CGMLGuiDlg : public CDialogEx
{
// Construction
public:
	CGMLGuiDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_GMLGUI_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation

	GraphicNotifierContainer* graphicResultWindow;

public:
	BContainer	wndContainer;
	void ComboCallBack();
	void AddConnector();
	void RemoveConnector();
	bool StartGmlExec(char* algorithmName,GML::Utils::AttributeList*	attrList);
	
	bool AlgorithmExecute(char* algorithmName,GML::Utils::AttributeList*	attrList );
protected:
	HICON m_hIcon;
	
	
	
	
	bool FindAlgorithms();
	
	bool DrawSelectedAlgorithm(char* configurationFile=NULL);
	bool LoadAlgorithm(char* algorithmName,char* configurationFile);
	bool AddBool(GML::Utils::Attribute* attr, bool boolValue,unsigned char offset = 1);
	bool AddUInt(GML::Utils::Attribute* attr, UINT uint32Value,unsigned char offset = 1);
	bool AddInt(GML::Utils::Attribute* attr, UINT int32Value,unsigned char offset = 1);
	bool AddDouble(GML::Utils::Attribute* attr, double value,unsigned char offset = 1);
	bool AddString(GML::Utils::Attribute* attr,char* value,unsigned char offset = 1,char valueType=GML::Utils::AttributeList::STRING);
	bool AddSpecialCombo(char* comboName,char* extension,GML::Utils::Attribute* attr,GML::Utils::GString& str,unsigned char offset = 1);
	bool AddSelectionCombo(char* comboName,char* btnStr, char* extension,GML::Utils::Attribute* attr,GML::Utils::GString& str,unsigned char offset = 1);
	bool AddConnector(BSelection* currentSelection,GML::Utils::GString& connectorStr);
	bool AddFilesFromDirToCombo(char* dirName,char* extension, BCombo* currentCombo);
	bool AddChildrenFromObject(BItem* currentProperty,int* pNewValue,GML::Utils::AttributeList* newAttrList);
	bool CreateChildren(BItem* currentProperty,GML::Utils::AttributeList&	attrList);
	bool AddMissingAttributes(char* pluginName,GML::Utils::AttributeList* newAttrList, GML::Utils::AttributeList* finalList);
	//bool AddChildrenForSelectionObject(BSelection* selectionObj,int* pNewValue,GML::Utils::AttributeList* newAttrList);
	bool AddChildrenForSelectionObject(BSelection* selectionObj,char* str,GML::Utils::AttributeList* newAttrList);
	bool AlgorithmRun(char* algorithmName, GML::Utils::AttributeList*	attrList);
	
	
	//bool MoveElement(UINT newPos);

	

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnCbnSelchangeCmbAlgorithms();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CComboBox cmbAlgorithms;
	afx_msg void OnFileSaveConfiguration();
	afx_msg void OnFileLoadConfiguration();
	afx_msg void OnFileRun();
};
