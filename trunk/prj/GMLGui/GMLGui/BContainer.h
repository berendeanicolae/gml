#pragma once
#include "afxwin.h"
#include "GString.h"
#include "Vector.h"
#include "BContainerSizes.h"
#include "BEdit.h"
#include "BCombo.h"
#include "BFile.h"
#include "BHeader.h"
#include "BCheckBox.h"
#include "BSelection.h"
#include "BButton.h"
#include "gml\gmllib.h"



#define TYPE_COMBO 100
#define TYPE_HEADER 101
#define TYPE_ELEMENT_HEADER 102

class BContainer :
	public CWnd
{
private:
	CFont simpleFont;
	Vector vElements;
	CPoint m_vpo;
	DWORD lastId;;
	CString algorithmName;
	DWORD lastYpos;

public:
	
	bool SetAlgorithmName(char* algName);
	void ResetScrollBar();
	DWORD GetNrElements();
	BItem* GetElement(int i);
	bool AddElement(BItem* element);
	BEdit* AddEditBox(char* textBox, unsigned char offset, UINT id);
	BFile* AddFile(char* comboTxt,unsigned char offset, UINT id,bool isFolder=false);
	BSelection* AddSelection(char* comboTxt,char* btnTxt,unsigned char offset, UINT id = 0);
	bool AddHeader(char* headerName,UINT id);
	BCheckBox* AddCheckBox(char* comboTxt,unsigned char offset, UINT id);
	BButton* AddButton(char* str, char* buttonTxt,unsigned char offset, UINT id=0);
	BCombo* AddComboBox(char* comboTxt,unsigned char offset, UINT id);
	bool SetCollapsedState(int id, int state);
	bool ShowElement(int id,int cmdShow);
	void ShowChildren();
	void Reset();
	bool DeleteChildren(BItem* currentProperty);
	bool DeleteElement(BItem* currentProperty);
	bool MoveElement(BItem* el, unsigned int newPos);
	int GetElementPos(BItem* el);
	int GetLastChildPos(BItem* someEl);
	BItem* GetParent(unsigned int i);
	BItem* GetSelectedElement();
	bool SaveConfiguration(char* algorithmName, char* fileName,GML::Utils::AttributeList*	attrList= NULL);
	bool SaveConfigurationToAttributeList(char* algorithmName,GML::Utils::AttributeList*	attrList);
	bool HasChildren(BItem* el);
	bool GetConnectorsSaveString(int* i,CString* str);
	bool GetComplexElement(int* i, CString* str);
	bool GetConnectorStr(int* i,CString* tempConnector);
	bool GetElementStr(BItem* element,CString* str);
	
	
	

	BContainer(void);
	bool Create(unsigned int x, unsigned int y, unsigned int width, unsigned height,CWnd* parent,unsigned int ID=0);
	~BContainer(void);

	afx_msg void OnPaint();
	afx_msg void OnVScroll(UINT nSBCode,UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	

	DECLARE_MESSAGE_MAP();
};

