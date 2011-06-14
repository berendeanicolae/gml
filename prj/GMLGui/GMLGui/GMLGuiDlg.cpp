
// GMLGuiDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GMLGui.h"
#include "GMLGuiDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif	

//
// ++++ CALLBACKS
//
void SelchangeCallBack(void* a)
{
	CGMLGuiDlg* mainWindow = (CGMLGuiDlg*)a;
	mainWindow->ComboCallBack();
}

void btnClickAddConnector(void *a)
{
	CGMLGuiDlg* mainWindow = (CGMLGuiDlg*)a;
	mainWindow->AddConnector();
}

void removeConnector(void *a)
{
	CGMLGuiDlg* mainWindow = (CGMLGuiDlg*)a;
	mainWindow->RemoveConnector();
}

//
//
//


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CGMLGuiDlg dialog




CGMLGuiDlg::CGMLGuiDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGMLGuiDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGMLGuiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CMB_ALGORITHMS, cmbAlgorithms);
}

BEGIN_MESSAGE_MAP(CGMLGuiDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_SELCHANGE(IDC_CMB_ALGORITHMS, &CGMLGuiDlg::OnCbnSelchangeCmbAlgorithms)
	ON_COMMAND(ID_FILE_SAVECONFIGURATION, &CGMLGuiDlg::OnFileSaveConfiguration)
	ON_COMMAND(ID_FILE_LOADCONFIGURATION, &CGMLGuiDlg::OnFileLoadConfiguration)
	ON_COMMAND(ID_FILE_RUN, &CGMLGuiDlg::OnFileRun)
END_MESSAGE_MAP()


// CGMLGuiDlg message handlers

BOOL CGMLGuiDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	graphicResultWindow = NULL;
	CRect rect;

	GetWindowRect(&rect);

	if(!wndContainer.Create(20,50,rect.Width() - 40 ,rect.Height()-170,this))
	{
		AfxMessageBox(_T("failed to create window \n"));
		ExitProcess(-1);
	}

	if(!FindAlgorithms())
	{
		MessageBox(_T("There was an error getting algorithms"),_T("Error Retreaving Algorithms"),MB_OK | MB_ICONERROR);
		ExitProcess(-1);
	}


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CGMLGuiDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGMLGuiDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGMLGuiDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


bool CGMLGuiDlg::FindAlgorithms()
{
	WIN32_FIND_DATA lpFindData;
	TCHAR currentPath[MAX_PATH];
	HANDLE hFind;
	DWORD size;
	unsigned int lastSlash = 0;

	size = GetModuleFileName(NULL,currentPath,MAX_PATH);
	if (size == 0 || GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		return false;
	for(unsigned int i=0;currentPath[i]!=_T('\0');i++)
	{
		if(currentPath[i] == _T('\\'))
		{
			lastSlash = i;
		}
	}

	currentPath[lastSlash+1] = _T('\0');
	_tcscat(currentPath,_T("Algorithms\\*.alg"));

	hFind = FindFirstFile(currentPath,&lpFindData);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;
	do 
	{
		if((lpFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			//doar fisiere ;)
			size = _tcslen(lpFindData.cFileName);
			if(size > 4)
			{
				//asa ar trebui ca doar mi-a gasit tot ce face match la *.alg
				if(lpFindData.cFileName[size-1] == _T('g')
					&& lpFindData.cFileName[size-2] == _T('l')
					&& lpFindData.cFileName[size-3] == _T('a')
					&& lpFindData.cFileName[size-4] == _T('.'))
				{
					lpFindData.cFileName[size-4] = _T('\0');
					cmbAlgorithms.AddString(lpFindData.cFileName);
				}
			}
		}
	} while (FindNextFile(hFind,&lpFindData));

	FindClose(hFind);
	return true;


}

void CGMLGuiDlg::OnCbnSelchangeCmbAlgorithms()
{
	wndContainer.Reset();
	
	DrawSelectedAlgorithm();
	wndContainer.ShowChildren();
}


bool CGMLGuiDlg::DrawSelectedAlgorithm(char* configurationFile)
{
	unsigned int curSel;
	CString itemText;
	
	curSel = cmbAlgorithms.GetCurSel();
	if(curSel != CB_ERR)
	{
		cmbAlgorithms.GetLBText(curSel,itemText);
		//ca sa ma ajute sa misc proprietatile intre ele
		

	
		//asta il face sa se deseneze	
		if(!LoadAlgorithm((char*)itemText.GetString(),configurationFile))
		{
			return false;
		}
	
	}
	return true;
}


bool CGMLGuiDlg::AddBool(GML::Utils::Attribute* attr, bool boolValue,unsigned char offset)
{
	GML::Utils::GString			str;
	GML::Utils::GString			objectDescription;
	GML::Utils::GString			objectMetadataList;
	BCheckBox*					checkBox;


	checkBox = wndContainer.AddCheckBox(attr->Name,offset,0);
	if(checkBox== NULL)
		return false;
	if(!attr->GetDescription(objectDescription))
		return false;

	
	checkBox->SetCheck(boolValue);
	if(!checkBox->SetDescription(objectDescription.GetText()))
		return false;
	if(!checkBox->SetMetaData(attr->MetaData))
		return false;
	return true;

}

bool CGMLGuiDlg::AddString(GML::Utils::Attribute* attr,char* value,unsigned char offset, char valueType)
{
	GML::Utils::GString			str;
	GML::Utils::GString			objectDescription;
	GML::Utils::GString			objectMetadataList,textItem;
	BEdit*						editBox;
	BCombo*						comboBox;
	BFile*						fileTxt;
	int							currentPos = 0,selectedItem = 0, numberOfItems = 0;



	
	attr->GetListItems(objectMetadataList);
	
	if(!attr->GetDescription(objectDescription))
		return false;

	if(objectMetadataList.GetSize() > 0)
	{
		
		
		comboBox = wndContainer.AddComboBox(attr->Name,offset,0);
		if(comboBox== NULL)
			return false;

		while(objectMetadataList.CopyNext(&textItem,",",&currentPos))
		{
			if(textItem.Compare(value,true)==0)
			{
				selectedItem = numberOfItems;
			}
			comboBox->AddString(textItem.GetText());
			numberOfItems+=1;
		}

		comboBox->SetSelectedItem(selectedItem);
		comboBox->elementType = TYPE_COMBO;
		comboBox->SetMetaData(attr->MetaData);
		return true;
	
	}
	if(attr->GetFlags() & GML::Utils::AttributeFlags::FL_FILEPATH || attr->GetFlags() & GML::Utils::AttributeFlags::FL_FOLDER)
	{
		fileTxt = wndContainer.AddFile(attr->Name,offset,0,(attr->GetFlags() & GML::Utils::AttributeFlags::FL_FOLDER)!=0);
		fileTxt->SetText(value);
		if(!fileTxt->SetDescription(objectDescription.GetText()))
			return false;
		fileTxt->elementType =  TYPE_FILE;
		return true;
	}
	
		
	editBox = wndContainer.AddEditBox(attr->Name,offset,0);
	
	if(editBox== NULL)
		return false;

	editBox->SetText(value);
	editBox->elementType =  valueType;
	
	if(!editBox->SetDescription(objectDescription.GetText()))
		return false;


	if(!editBox->SetMetaData(attr->MetaData))
		return false;
	return true;

}


bool CGMLGuiDlg::AddInt(GML::Utils::Attribute* attr, UINT int32Value,unsigned char offset)
{
	char strValue[100];
		
	sprintf_s(strValue,100,"%d",int32Value);

	return AddString(attr,strValue,offset,GML::Utils::AttributeList::INT32);
}

bool CGMLGuiDlg::AddUInt(GML::Utils::Attribute* attr, UINT uint32Value,unsigned char offset)
{
	char strValue[100];
		
	sprintf_s(strValue,100,"%u",uint32Value);
	return AddString(attr,strValue,offset,GML::Utils::AttributeList::UINT32);

}

bool CGMLGuiDlg::AddDouble(GML::Utils::Attribute* attr, double dvalue,unsigned char offset)
{
	char strValue[100];
		
	sprintf_s(strValue,100,"%lf",dvalue);

	return AddString(attr,strValue,offset,GML::Utils::AttributeList::DOUBLE);

}


bool CGMLGuiDlg::LoadAlgorithm(char* algorithmName,char* configurationFile)
{
	GML::Algorithm::IAlgorithm	*alg = NULL;
	GML::Utils::AttributeList	attrList;
	GML::Utils::Attribute		*attr;
	GML::Utils::GString			str;
	GML::Utils::GString			objectDescription;
	GML::Utils::GString			objectMetadataList;


	CString						temp;
	CString						description;
	unsigned int				strResult;


	bool						boolValue;
	UInt32						uint32Value;
	Int32						int32Value;
	double						doubleValue;



	if(algorithmName == NULL)
	{
		return false;
	}

	if ((alg = GML::Builder::CreateAlgorithm(algorithmName))==NULL)
		return false;
	
	if(configurationFile != NULL)
	{
		if(!attrList.Load(configurationFile))
			return false;
	}
	
	
	if (alg->GetProperty(attrList)==false)
		return false;

	if(configurationFile != NULL)
	{
		if(!attrList.Load(configurationFile))
			return false;
	}
	
	wndContainer.AddHeader(algorithmName,0);
	for (int tr=0;tr<attrList.GetCount();tr++)
	{
		attr = attrList.Get(tr);
		
		printf("%s = ",attr->Name);

		switch (attr->AttributeType)
		{
		case GML::Utils::AttributeList::BOOLEAN:
			attrList.UpdateBool(attr->Name,boolValue);
			if(!AddBool(attr, boolValue)){return false;}
			break;

		case GML::Utils::AttributeList::UINT32:

			attrList.UpdateUInt32(attr->Name,uint32Value);
			if(!AddUInt(attr,uint32Value)){return false;}
			break;	
		case GML::Utils::AttributeList::INT32:
			
			attrList.UpdateInt32(attr->Name,int32Value);
			if(!AddInt(attr,int32Value)){return false;}
			break;	
		case GML::Utils::AttributeList::DOUBLE:
			attrList.UpdateDouble(attr->Name,doubleValue);
			if(!AddDouble(attr,doubleValue)){return false;}
			break;	
		case GML::Utils::AttributeList::STRING:

	
			attrList.UpdateString(attr->Name,str);
			//verific daca nu e unul din stringurile din combo
				
			
			strResult = _tcsicmp(attr->Name,_T("Connector"));
			if(strResult == 0)
			{
				//seteaza Connectors;
				if(!AddSelectionCombo("Connectors"," Add ","dbc",attr,str)){return false;}
				break;
			}
			
			strResult = _tcsicmp(attr->Name,_T("DataBase"));
			if(strResult == 0)
			{
			
				if(!AddSpecialCombo("DataBases","db",attr,str)){return false;}
				break;
			}

			strResult = _tcsicmp(attr->Name,_T("Notifier"));
			if(strResult == 0)
			{
				if(!AddSpecialCombo("Notifiers","ntf",attr,str)){return false;}
				break;
			}
									
			//verific daca nu are elemente de tip List, daca e asa, inseamna ca treubie sa bag un combo box
			//pur si simplu afisez stringul
			if(!AddString(attr, str)){return false;}
			break;

		};
	}

	return true;
	
}


bool CGMLGuiDlg::CreateChildren(BItem* currentProperty,GML::Utils::AttributeList&	attrList)
{
	CString						temp;
	GML::Utils::Attribute		*attr;
	GML::Utils::GString			str;
	GML::Utils::GString			objectDescription;
	GML::Utils::GString			objectMetadataList;
	int							currentPos;
	BItem*						createdProperty;

	bool						boolValue;
	UInt32						uint32Value;
	Int32						int32Value;
	double						doubleValue;

	unsigned int				parentOffset;

	
	if(!wndContainer.DeleteChildren(currentProperty))
		return false;

	currentPos = wndContainer.GetElementPos(currentProperty);
	if(currentPos < 0)
		return false;
	parentOffset = currentProperty->offset;
	//adaug de la urmatoarea pozitie.
	currentPos+=1;

	for (int tr=0;tr<attrList.GetCount();tr++)
	{
		attr = attrList.Get(tr);

		switch (attr->AttributeType)
		{	

		case GML::Utils::AttributeList::BOOLEAN:
			attrList.UpdateBool(attr->Name,boolValue);
			if(!AddBool(attr, boolValue,parentOffset+1))
				return false;
			createdProperty = wndContainer.GetElement(wndContainer.GetNrElements()-1);
			if(createdProperty == NULL){return false;}
			wndContainer.MoveElement(createdProperty,currentPos);
			currentPos++;

			break;

		case GML::Utils::AttributeList::UINT32:
			attrList.UpdateUInt32(attr->Name,uint32Value);
			if(!AddUInt(attr, uint32Value,parentOffset+1))
				return false;
			createdProperty = wndContainer.GetElement(wndContainer.GetNrElements()-1);
			if(createdProperty == NULL){return false;}
			wndContainer.MoveElement(createdProperty,currentPos);
			currentPos++;
			break;	
		case GML::Utils::AttributeList::INT32:
			attrList.UpdateInt32(attr->Name,int32Value);
			if(!AddInt(attr, int32Value,parentOffset+1))
				return false;
			createdProperty = wndContainer.GetElement(wndContainer.GetNrElements()-1);
			if(createdProperty == NULL){return false;}
			wndContainer.MoveElement(createdProperty,currentPos);
			currentPos++;
			break;	

		case GML::Utils::AttributeList::DOUBLE:
			attrList.UpdateDouble(attr->Name,doubleValue);
			if(!AddDouble(attr, doubleValue,parentOffset+1))
				return false;
			createdProperty = wndContainer.GetElement(wndContainer.GetNrElements()-1);
			if(createdProperty == NULL){return false;}
			wndContainer.MoveElement(createdProperty,currentPos);
			currentPos++;
			break;	
		case GML::Utils::AttributeList::STRING:
			attrList.UpdateString(attr->Name,str);
			//pur si simplu afisez stringul
			if(!AddString(attr, str.GetText(),parentOffset+1))
				return false;
			createdProperty = wndContainer.GetElement(wndContainer.GetNrElements()-1);
			if(createdProperty == NULL){return false;}
			wndContainer.MoveElement(createdProperty,currentPos);
			currentPos++;
			break;
		};
	}
	return true;
}

bool CGMLGuiDlg::AddMissingAttributes(char* pluginName,GML::Utils::AttributeList* newAttrList, GML::Utils::AttributeList* finalList)
{
	GML::Utils::AttributeList	attrList;
	GML::Utils::Attribute* currentAttribute,*testAttribute;

	if(!GML::Builder::GetPluginProperties(pluginName,attrList))
		return false;
	for(int i=0;i<attrList.GetCount();i++)
	{
		currentAttribute = attrList.Get(i);
		testAttribute = newAttrList->Get(currentAttribute->Name);
		if(testAttribute == NULL)
		{
			finalList->AddAttribute(currentAttribute->Name,currentAttribute->Data,currentAttribute->AttributeType,currentAttribute->ElementsCount,currentAttribute->MetaData);
			continue;
		}

		switch(currentAttribute->AttributeType)
		{
		case GML::Utils::AttributeList::BOOLEAN:
			bool temp;
			if(testAttribute->DataSize>=4 && stricmp((char*)testAttribute->Data,"True")==0)
			{
				temp = true;
				finalList->AddBool(testAttribute->Name,temp,currentAttribute->MetaData);
			}
			else if (testAttribute->DataSize>=5 && stricmp((char*)testAttribute->Data,"False")==0)
			{
				temp = false;
				finalList->AddBool(testAttribute->Name,temp,currentAttribute->MetaData);
			}
			break;

		case GML::Utils::AttributeList::UINT32:
			unsigned int tempUInt;
			tempUInt = atoi((char*)testAttribute->Data);
			finalList->AddUInt32(testAttribute->Name,tempUInt,currentAttribute->MetaData);
			break;
		case GML::Utils::AttributeList::INT32:
			int tempInt;
			tempInt = atoi((char*)testAttribute->Data);
			finalList->AddInt32(testAttribute->Name,tempInt,currentAttribute->MetaData);
			break;	
		case GML::Utils::AttributeList::DOUBLE:
			_CRT_DOUBLE tempDouble;

			if(_atodbl(&tempDouble,(char*)testAttribute->Data)==0)
				finalList->AddDouble(currentAttribute->Name,tempDouble.x,currentAttribute->MetaData);
			break;	
		case GML::Utils::AttributeList::STRING:
			finalList->AddString(currentAttribute->Name,(char*)testAttribute->Data,currentAttribute->MetaData);	
			break;
		}
				
	}

	return true;
}
bool CGMLGuiDlg::AddChildrenForSelectionObject(BSelection* selectionObj,char* str,GML::Utils::AttributeList* newAttrList)
{
	GML::Utils::AttributeList	attrList;
	GML::Utils::AttributeList	finalList;
	GML::Utils::Attribute* currentAttribute,*testAttribute;
	BButton* btnElement;
	CString selectedString;
	DWORD elementPos;
	unsigned int offset;

	offset = selectionObj->offset+1;
	elementPos = wndContainer.GetLastChildPos(selectionObj);
	btnElement = wndContainer.AddButton(str,"Remove",offset);
	btnElement->SetCallbackBtnClick(&removeConnector,this);
	wndContainer.MoveElement(btnElement,elementPos+1);
	
	if(!GML::Builder::GetPluginProperties(str,attrList))
		return false;
	if(btnElement == NULL)
		return false;

	if(newAttrList != NULL)
	{
		if(!AddMissingAttributes(str,newAttrList, &finalList))
			return false;
		if(!CreateChildren(btnElement,finalList))
		{
			return false;
		}
		return true;
	}

	if(!CreateChildren(btnElement,attrList))
	{
		return false;
	}	
	
	return true;
}

/*
bool CGMLGuiDlg::AddChildrenForSelectionObject(BSelection* selectionObj,int* pNewValue,GML::Utils::AttributeList* newAttrList)
{


	selectionObj->GetItem(*pNewValue,selectedString);
	


}
*/

bool CGMLGuiDlg::AddChildrenFromObject(BItem* currentProperty,int* pNewValue,GML::Utils::AttributeList* newAttrList)
{

	BCombo* objectCombo;
	GML::Utils::AttributeList	attrList;
	GML::Utils::AttributeList	finalList;
	GML::Utils::Attribute* currentAttribute,*testAttribute;
	CString selectedString;
	unsigned int offset;

	objectCombo = (BCombo*)currentProperty;
	objectCombo->GetItem(*pNewValue,selectedString);
	offset = objectCombo->offset+1;
	GML::Builder::GetPluginProperties((char*)((selectedString).GetString()),attrList);

	if(newAttrList != NULL)
	{
		/*
		for(int i=0;i<attrList.GetCount();i++)
		{
			currentAttribute = attrList.Get(i);
			testAttribute = newAttrList->Get(currentAttribute->Name);
			if(testAttribute == NULL)
			{
				finalList.AddAttribute(currentAttribute->Name,currentAttribute->Data,currentAttribute->AttributeType,currentAttribute->ElementsCount,currentAttribute->MetaData);
				continue;
			}

			switch(currentAttribute->AttributeType)
			{
			case GML::Utils::AttributeList::BOOLEAN:
				bool temp;
				if(testAttribute->DataSize>=4 && stricmp((char*)testAttribute->Data,"True")==0)
				{
					temp = true;
					finalList.AddBool(testAttribute->Name,temp,currentAttribute->MetaData);
				}
				else if (testAttribute->DataSize>=5 && stricmp((char*)testAttribute->Data,"False")==0)
				{
					temp = false;
					finalList.AddBool(testAttribute->Name,temp,currentAttribute->MetaData);
				}
				break;

			case GML::Utils::AttributeList::UINT32:
				unsigned int tempUInt;
				tempUInt = atoi((char*)testAttribute->Data);
				finalList.AddUInt32(testAttribute->Name,tempUInt,currentAttribute->MetaData);
				break;
			case GML::Utils::AttributeList::INT32:
				int tempInt;
				tempInt = atoi((char*)testAttribute->Data);
				finalList.AddInt32(testAttribute->Name,tempInt,currentAttribute->MetaData);
				break;	
			case GML::Utils::AttributeList::DOUBLE:
				_CRT_DOUBLE tempDouble;

				if(_atodbl(&tempDouble,(char*)testAttribute->Data)==0)
					finalList.AddDouble(currentAttribute->Name,tempDouble.x,currentAttribute->MetaData);
				break;	
			case GML::Utils::AttributeList::STRING:
				finalList.AddString(currentAttribute->Name,(char*)testAttribute->Data,currentAttribute->MetaData);	
				break;
			}
				
		}
	*/
		if(!AddMissingAttributes((char*)((selectedString).GetString()),newAttrList, &finalList))
			return false;
		if(!CreateChildren(currentProperty,finalList))
		{
			return false;
		}
		return true;
	}

	if(!CreateChildren(currentProperty,attrList))
	{
		return false;
	}	
	
	return true;


}

bool CGMLGuiDlg::AddFilesFromDirToCombo(char* dirName,char* extension, BCombo* currentCombo)
{
	WIN32_FIND_DATA lpFindData;
	TCHAR currentPath[MAX_PATH];
	HANDLE hFind;
	DWORD size;
	DWORD extensionSize;
	unsigned int lastSlash = 0;
	bool foundExtension;

	size = GetModuleFileName(NULL,currentPath,MAX_PATH);
	if (size == 0 || GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		return false;
	for(unsigned int i=0;currentPath[i]!=_T('\0');i++)
	{
		if(currentPath[i] == _T('\\'))
		{
			lastSlash = i;
		}
	}

	currentPath[lastSlash+1] = _T('\0');
	_tcscat(currentPath,dirName);
	_tcscat(currentPath,"\\*.");
	_tcscat(currentPath,extension);

	extensionSize = _tcslen(extension);

	hFind = FindFirstFile(currentPath,&lpFindData);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;
	do 
	{
		if((lpFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			//doar fisiere ;)
			size = _tcslen(lpFindData.cFileName);

			if(size > 4 && extensionSize <4)
			{
				//asa ar trebui ca doar mi-a gasit tot ce face match la *.alg
				foundExtension = true;
				for(unsigned int i=0; i< extensionSize; i++)
				{
					if(lpFindData.cFileName[size-i-1] != extension[extensionSize -i-1])
					{
						foundExtension = false;
						break;
					}
				}
				if(lpFindData.cFileName[size-extensionSize -1] != _T('.'))
				{
					foundExtension = false;
				}

				if(foundExtension)
				{
					lpFindData.cFileName[size-extensionSize -1] = _T('\0');
					currentCombo->AddString(lpFindData.cFileName);
				}

			}
		}
	} while (FindNextFile(hFind,&lpFindData));

	FindClose(hFind);
	return true;
};


void CGMLGuiDlg::AddConnector()
{
	int selectedSelectionElement;
	CString sel;
	
	//bool CGMLGuiDlg::AddChildrenFromObject(BItem* currentProperty,int* pNewValue,GML::Utils::AttributeList* newAttrList)
	BItem* selElement = wndContainer.GetSelectedElement();
	BSelection* currentSel = (BSelection*)selElement;
	//selectedSelectionElement = currentSel->GetSelectedElement();
	if(!currentSel->GetSelectedItem(sel))
		return;
	
	//AddChildrenForSelectionObject(currentSel,&selectedSelectionElement,NULL);
	AddChildrenForSelectionObject(currentSel,sel.GetBuffer(),NULL);
	wndContainer.ShowChildren();
}

void CGMLGuiDlg::RemoveConnector()
{
	int selectedSelectionElement;
	
	//bool CGMLGuiDlg::AddChildrenFromObject(BItem* currentProperty,int* pNewValue,GML::Utils::AttributeList* newAttrList)
	BItem* selElement = wndContainer.GetSelectedElement();
	wndContainer.DeleteElement(selElement);
	wndContainer.ShowChildren();
	
}


void CGMLGuiDlg::ComboCallBack()
{
	int selectedComboElement;
	
	//bool CGMLGuiDlg::AddChildrenFromObject(BItem* currentProperty,int* pNewValue,GML::Utils::AttributeList* newAttrList)
	BItem* selElement = wndContainer.GetSelectedElement();
	BCombo* currentCombo = (BCombo*)selElement;
	selectedComboElement = currentCombo->GetSelectedElement();
	
	AddChildrenFromObject(selElement,&selectedComboElement,NULL);
	wndContainer.ShowChildren();

}

bool CGMLGuiDlg::AddConnector(BSelection* currentSelection,GString& connectorStr)
{
	int pos,nrItems;
	GString connectorName;
	GString temp;
	GString						connectorValue[2];
	GML::Utils::AttributeList	loadedAtrList;
	
	
	pos = connectorStr.Find("{",true);
	if(pos < 0)
		return false;
	connectorName.Set("");
	connectorName.Add(connectorStr,pos);
	pos++;
	//pos = connectors[i].Find(";",true,pos);
	while(connectorStr.CopyNext(&temp,";",&pos,true))
	{
		if(!temp.Split("=",connectorValue,2,&nrItems)|| (nrItems !=2))
			continue;
		loadedAtrList.AddString(connectorValue[0],connectorValue[1]);
	}
	if(!AddChildrenForSelectionObject(currentSelection,connectorName,&loadedAtrList))
		return false;
	return true;
}
bool CGMLGuiDlg::AddSelectionCombo(char* comboName,char* btnStr, char* extension,GML::Utils::Attribute* attr,GML::Utils::GString& str,unsigned char offset)
{
	GML::Utils::GString			objectDescription;
	GML::Utils::GString			selectedObject;
	GML::Utils::GString			propertyName;
	GML::Utils::GString			propData[2];
	int							selectedIndex;
	GML::Utils::AttributeList	objectProperties;
	BSelection*					currentSelection;
	CString						el;
	GString						parser;
	GString						connectors[MAX_CONNECTORS]; //maxim 50 de conectori
	GML::Utils::Attribute		connectorsAttr[MAX_CONNECTORS];
	
	GString						temp;
	GString						connectorName;
	int							nrConnectors = 0;
	int pos;
	

	if(comboName == NULL)
		return false;
	if(extension == NULL)
		return false;

	currentSelection = (BSelection*)wndContainer.AddSelection(comboName,btnStr,offset,0);
	if(currentSelection == NULL)
		return false;
	if(!currentSelection->SetCallbackBtnClick(btnClickAddConnector,this))
		return false;
	if(!currentSelection->SetMetaData(attr->MetaData))
		return false;
	if(!attr->GetDescription(objectDescription))
		return false;
	if(!currentSelection->SetDescription(objectDescription.GetText()))
		return false;

	if(!AddFilesFromDirToCombo(comboName,extension,currentSelection))
	{
		currentSelection->AddString("error retrieving elements");
	}


	parser.Set((char*)attr->Data);
	if(attr->DataSize > 0)
	{
		if(parser.Split("=>",connectors,MAX_CONNECTORS,&nrConnectors) && nrConnectors > 0)
		{
		
			for(int i=0;i<nrConnectors;i++)
			{		
				AddConnector(currentSelection,connectors[i]);
			}
		}
		else
		{
			temp.Set((char*)attr->Data,attr->DataSize);
			AddConnector(currentSelection,temp);
		}
		wndContainer.ShowChildren();
	}
	
	return true;
	//parseaza connectorii si insereaza cate unul penru fiecare
	
	

}

bool CGMLGuiDlg::AddSpecialCombo(char* comboName,char* extension,GML::Utils::Attribute* attr,GML::Utils::GString& str,unsigned char offset)
{
	
	GML::Utils::GString			objectDescription;
	GML::Utils::GString			selectedObject;
	GML::Utils::GString			propertyName;
	GML::Utils::GString			propData[2];
	int							selectedIndex;
	GML::Utils::AttributeList	objectProperties;
	BCombo*						currentCombo;
	CString						el;
	

	if(comboName == NULL)
		return false;
	if(extension == NULL)
		return false;

	currentCombo = (BCombo*)wndContainer.AddComboBox(comboName,offset,0);
	if(currentCombo == NULL)
		return false;
	if(!currentCombo->SetCallbackCbnSelChange(SelchangeCallBack, this))
		return false;
	if(!currentCombo->SetMetaData(attr->MetaData))
		return false;
	
	if(!attr->GetDescription(objectDescription))
		return false;

	if(!currentCombo->SetDescription(objectDescription.GetText()))
		return false;

	
	if(!AddFilesFromDirToCombo(comboName,extension,currentCombo))
	{
		currentCombo->AddString("error retrieving elements");
	}

	
	//incarca intai obiectul setat
	selectedIndex = -1;
	int pos = 0;
	if(str.GetSize() > 1)
	{
		str.Strip(" \t\n\r\"");
		str.CopyNext(&selectedObject,"{",&pos);
		if(selectedObject.GetSize() > 0)
		{
			for(int i=0;i<currentCombo->GetNrItems();i++)
			{
				currentCombo->GetItem(i,el);
				if(selectedObject.Compare(el.GetBuffer(),true) == 0)
				{
					selectedIndex = i;
					break;
				}
					
			}
			
			if(selectedIndex != -1)
			{
				//construieste lista de atribute
				
				if(str.GetText()[str.GetSize()-1] == '}')
					str.ReplaceOnPos(str.GetSize()-2,str.GetSize()-1,";");

				//
				currentCombo->SetSelectedItem(selectedIndex);
				while(str.CopyNext(&propertyName,";",&pos))
				{
					
					if(propertyName.Contains("=",false) && propertyName.Split("=",propData,2))
					{
						propData[0].Strip();
						propData[1].Strip();
						objectProperties.AddString(propData[0],propData[1]);
						
					}
				}

				if(!AddChildrenFromObject(currentCombo,&selectedIndex,&objectProperties))
					return false;

			}
		}
	}


	return true;
}

void CGMLGuiDlg::OnFileSaveConfiguration()
{
	
	int i;
	CString fileName;
	CString selectedAlgorithm;

	i = cmbAlgorithms.GetCurSel();
	if(i < 0)
		return;
	cmbAlgorithms.GetLBText(i,selectedAlgorithm);

	
	CFileDialog dlg(FALSE/*Open=TRUE Save=False*/,
					"ia"/*Filename Extension*/,
					selectedAlgorithm/*Initial Filename*/,
					OFN_ENABLESIZING|OFN_EXPLORER/*Flags*/,
					"Invatare Automata(*.ia)|"/*Filetype Filter*/,
					this/*parent Window*/);
	if (dlg.DoModal() == IDOK)
	{
		fileName = dlg.GetPathName();
		if(!wndContainer.SaveConfiguration((char*)selectedAlgorithm.GetString(),(char*)fileName.GetString()))
		{
			AfxMessageBox("Sorry, but the configuration could not be saved");
			return;
		}
		AfxMessageBox("The configuration was saved saved successfully!",MB_OK | MB_ICONINFORMATION);

	}
}


void CGMLGuiDlg::OnFileLoadConfiguration()
{
	GML::Utils::AttributeList atributeList;
	GML::Utils::Attribute* algName;
	char* algorithmName;
	CString fileName = "D:\\Shared VM\\share-write\\temp\\testGdt\\test2\\gml\\template2.txt";
	CString someAlgorithm;
	

//	fileName = currentAlgorithm->algorithmName;
	CFileDialog dlg(true/*Open=TRUE Save=False*/,
		"ia"/*Filename Extension*/,
		NULL/*Initial Filename*/,
		OFN_ENABLESIZING|OFN_EXPLORER|OFN_FILEMUSTEXIST/*Flags*/,
		"Invatare Automata(*.ia)|"/*Filetype Filter*/,
		this/*parent Window*/);
	if (dlg.DoModal() == IDOK)
	{
		fileName = dlg.GetPathName();
		

		if(!atributeList.Load((char*)fileName.GetString()))
		{
			AfxMessageBox("Invalid file.",MB_OK | MB_ICONERROR);
			return;
		}

		algName = atributeList.Get("AlgorithmName");
		if(algName == NULL)
		{
			AfxMessageBox("Invalid file. Failed to find algorithm name",MB_OK | MB_ICONERROR);
			return;
		}

		if(algName->AttributeType != GML::Utils::AttributeList::STRING)
		{
			AfxMessageBox("Invalid file. Failed to get algorithm name",MB_OK | MB_ICONERROR);
			return;
		}

		algorithmName = (char*)algName->Data;


		for(int i=0;i<cmbAlgorithms.GetCount();i++)
		{
			cmbAlgorithms.GetLBText(i,someAlgorithm);
			if(someAlgorithm.CompareNoCase(algorithmName)==0)
			{
				//am gasit algoritmul
				cmbAlgorithms.SetCurSel(i);
				wndContainer.Reset();
				DrawSelectedAlgorithm((char*)fileName.GetString());
				wndContainer.ShowChildren();
				return;
			}
		}

		AfxMessageBox("Failed to find specified algorithm in current loaded algorithms.",MB_OK | MB_ICONERROR);
		return ;

	}
}


bool CGMLGuiDlg::StartGmlExec(char* algorithmName,GML::Utils::AttributeList*	attrList)
{
	WIN32_FIND_DATA lpFindData;
	TCHAR currentPath[MAX_PATH];
	TCHAR tempFileName[MAX_PATH];
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	HANDLE hFind;
	DWORD size;
	unsigned int lastSlash = 0;
	CString parameters;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );
	this->SetFocus();

	size = GetModuleFileName(NULL,currentPath,MAX_PATH);
	if (size == 0 || GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		return false;
	for(unsigned int i=0;currentPath[i]!=_T('\0');i++)
	{
		if(currentPath[i] == _T('\\'))
		{
			lastSlash = i;
		}
	}

	currentPath[lastSlash+1] = _T('\0');


	sprintf(tempFileName,"%stemp\\gmlExecTempFile.tmp",currentPath);
	_tcscat(currentPath,_T("GMLExec.exe"));

	//salveaza configuratia intr-un fisier


	if(!wndContainer.SaveConfiguration(algorithmName,tempFileName,attrList))
	{
		AfxMessageBox("Failed to save the configuration to temp path. Can not start running the algorithm");
		return false;	
	}


	parameters =currentPath;
	parameters+=" \"";
	parameters+=tempFileName;	
	parameters+="\"";


	// Start the child process. 
	if( !CreateProcess( NULL,   // No module name (use command line)
		(char*)parameters.GetString(),        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi )           // Pointer to PROCESS_INFORMATION structure
		) 
	{
		AfxMessageBox( "Failed to execute gmlExec.exe",MB_ICONERROR | MB_OK);
		return false;
	}
	// Wait until child process exits.
	WaitForSingleObject( pi.hProcess, INFINITE );

	// Close process and thread handles. 
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );

	return true;
}


void CGMLGuiDlg::OnFileRun()
{
	GML::Utils::AttributeList	attrList;
	GML::Utils::Attribute* notifier;
	CString	selectedAlgorithm;
	int i;
	CString fileName;

	i = cmbAlgorithms.GetCurSel();
	if(i < 0)
		return;
	cmbAlgorithms.GetLBText(i,selectedAlgorithm);

	
	if(!wndContainer.SaveConfigurationToAttributeList(selectedAlgorithm.GetBuffer(),&attrList))
	{
		AfxMessageBox("Failed to save the configuration. Can not start running the algorithm");
		return;
	}

	notifier = attrList.Get("Notifier");
	if(notifier == NULL)
	{
		AfxMessageBox("Failed to find notifier. Can not start running the algorithm");
		return;
	}


	if(strnicmp((char*)notifier->Data,"ConsoleNotifier",15)==0)
	{
		StartGmlExec(selectedAlgorithm.GetBuffer(),&attrList);
	}
	else if (strnicmp((char*)notifier->Data,"WrapperNotifier",15)==0)
	{
		if(!IsWindow(graphicResultWindow->GetSafeHwnd()))
			graphicResultWindow = NULL;

		if(graphicResultWindow == NULL)
		{
			graphicResultWindow = new GraphicNotifierContainer();
			graphicResultWindow->Create("Graphic Notifier",20,20,520,520,this,ID_ALGORITHM_CONTAINER);
		}	

		graphicResultWindow->AddAlgorithm(selectedAlgorithm.GetBuffer());
		AlgorithmRun(selectedAlgorithm.GetBuffer(),&attrList);
	}
	

}


bool CGMLGuiDlg::AlgorithmRun(char* algorithmName, GML::Utils::AttributeList*	attrList)
{
	GML::Algorithm::IAlgorithm	*alg;
	CString temp;
	
	if ((alg=GML::Builder::CreateAlgorithm(algorithmName))==NULL)
	{
		temp.Format("[ERROR] -> Error creating '%s'",algorithmName);
		AfxMessageBox(temp);
		return false;
	}
	if (alg->SetProperty(*attrList)==false)
	{
		temp.Format("[ERROR] -> Error updateing configuration to '%s'",algorithmName);
		AfxMessageBox(temp);
		return false;
	}
	if (alg->Init()==false)
	{
		temp.Format("[ERROR] -> Error on initialization of '%s'",algorithmName);
		AfxMessageBox(temp);
		return false;
	}
	if (alg->Execute()==false)
	{
		temp.Format("[ERROR] -> Error on execution of '%s'",algorithmName);
		AfxMessageBox(temp);
		return false;
	}
	//alg->Wait();
	//printf("Algorithm '%s' done !\n",currentAlgorithm->algorithmName);
	return true;
}
