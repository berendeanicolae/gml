
// GMLInstallerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GMLInstaller.h"
#include "GMLInstallerDlg.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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


// CGMLInstallerDlg dialog




CGMLInstallerDlg::CGMLInstallerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGMLInstallerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGMLInstallerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CGMLInstallerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(BTN_CANCEL, &CGMLInstallerDlg::OnBnClickedPanel1BtnCancel)
	ON_BN_CLICKED(IDC_PANEL1_BTN_AGREE, &CGMLInstallerDlg::OnBnClickedPanel1BtnAgree)
	ON_BN_CLICKED(ID_BUTTON_NEXT,&CGMLInstallerDlg::OnBnClickedNext)
	ON_BN_CLICKED(ID_BUTTON_BACK,&CGMLInstallerDlg::OnBnClickedBack)
	ON_BN_CLICKED(ID_BUTTON_BROWSE,&CGMLInstallerDlg::OnBnClickedBrowse)
	ON_BN_CLICKED(ID_BUTTON_FINISH,&CGMLInstallerDlg::OnOK)
	ON_NOTIFY(LVN_HOTTRACK,ID_LIST_COMPONENTS,OnComponentsListViewHoover)
	ON_NOTIFY(LVN_ITEMCHANGING,ID_LIST_COMPONENTS,OncomponentListViewChanged)
	ON_WM_CTLCOLOR()

END_MESSAGE_MAP()



// CGMLInstallerDlg message handlers

BOOL CGMLInstallerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	char filePath[MAX_PATH];

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

	//reseteaza vectorul de controale :)
	memset((void*)controls,0,MAX_CONTROL_NR*sizeof(ControlInfo));
	currentTab = TAB_EULA;
	nrControls = 0;

	// TODO: Add extra initialization here
	char* strEula = "License";
	CEdit* edt = (CEdit*) GetDlgItem(ID_PANEL_EDIT_EULA);
	edt->SetWindowTextA(strEula);
	edt->SetSel(1,3); //?
	//adauga controalele
	AddControl(GetDlgItem(ID_PANEL_EDIT_EULA),TAB_EULA);
	AddControl(GetDlgItem(ID_PANEL1_IMAGE),TAB_EULA);
	AddControl(GetDlgItem(IDC_PANEL1_EULA_INFO),TAB_EULA);
	AddControl(GetDlgItem(ID_PANEL_EDIT_EULA),TAB_EULA);
	AddControl(GetDlgItem(IDC_PANEL1_BTN_AGREE),TAB_EULA);

	//linia si butonul cancel
	AddControl(GetDlgItem(BTN_CANCEL),TAB_SHOWALWAYS);
	

	AddControl(GetDlgItem(IDC_LINE_ABOVE_CANCEL),TAB_SHOWALWAYS);

	greyDeschis.CreateSolidBrush(RGB(255,255,255));
	if(!GetModuleFileName(NULL,filePath,MAX_PATH))
	{
		MessageBox("Could not get module file name! Please run the installer in a path containing only ascii characters","Error loading GML",MB_ICONERROR | MB_OK);
		ExitProcess(-1);
	}
	
	DWORD overlayStart = GetOverlayStart(filePath);
	if(overlayStart == 0)
	{
		MessageBox("Could not locate current package.Possible cause is corrupt installer! Download and try again","Error loading GML",MB_ICONERROR | MB_OK);
		ExitProcess(-1);
	}

	//doar pentru debug
	//gmlPackage.Load("D:\\work-facultate\\GML\\trunk\\prj\\GMLInstaller\\GMLInstaller\\gml-package-r599.gdrp",0);
	
	
	if(!gmlPackage.Load(filePath,overlayStart))
	{
		MessageBox("Could not load GML package into memory. Possible cause may be corrupt installer! Download and try again","Error loading GML",MB_ICONERROR | MB_OK);
		ExitProcess(-1);
	}
	
	boldFont.CreateFontA(14,0,0,0,FW_BOLD,FALSE,FALSE,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,TEXT("Arial"));
	simpleFont.CreateFontA(14,0,0,0,FW_REGULAR,FALSE,FALSE,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,TEXT("Arial"));

	memset(intallComponents,FALSE,sizeof(intallComponents));

	AddComponentsTab();
	AddProgressTab();
	ShowControls(TAB_EULA);
	
	

	return TRUE;  // return TRUE  unless you set the focus to a control
}

DWORD CGMLInstallerDlg::GetOverlayStart(char* filePath)
{
	HANDLE hFile;
	unsigned char* buffer;
	DWORD nrBytesRead;
	IMAGE_DOS_HEADER* dh;
	IMAGE_NT_HEADERS* nh;
	IMAGE_SECTION_HEADER* sect;
	DWORD nrSections;
	DWORD overlayPos = 0;
	DWORD peHeaderPos = 0;
	

	hFile = CreateFile(filePath,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return 0;
	buffer = (unsigned char*)malloc(sizeof(IMAGE_DOS_HEADER));
	if(buffer == NULL){CloseHandle(hFile);return 0;}
	if(!ReadFile(hFile,buffer,sizeof(IMAGE_DOS_HEADER),&nrBytesRead,NULL) || nrBytesRead!= sizeof(IMAGE_DOS_HEADER)){free(buffer);CloseHandle(hFile);return 0;}
	dh = (IMAGE_DOS_HEADER*)buffer;
	if(INVALID_SET_FILE_POINTER == SetFilePointer(hFile,dh->e_lfanew,NULL,FILE_BEGIN)){free(buffer);CloseHandle(hFile);return 0;}
	peHeaderPos = dh->e_lfanew;
	free(buffer);
	//citesc nt_header
	buffer = (unsigned char*)malloc(sizeof(IMAGE_NT_HEADERS));
	if(buffer == NULL){CloseHandle(hFile);return 0;}
	if(!ReadFile(hFile,buffer,sizeof(IMAGE_NT_HEADERS),&nrBytesRead,NULL) || nrBytesRead!= sizeof(IMAGE_NT_HEADERS)){free(buffer);CloseHandle(hFile);return 0;}
	nh = (IMAGE_NT_HEADERS*)buffer;
	nrSections = nh->FileHeader.NumberOfSections;
	free(buffer);
	overlayPos = 0;

	buffer = (unsigned char*)malloc(sizeof(IMAGE_SECTION_HEADER)*nrSections);
	if(buffer == NULL){CloseHandle(hFile);return 0;}
	if(INVALID_SET_FILE_POINTER == SetFilePointer(hFile,peHeaderPos+sizeof(IMAGE_NT_HEADERS),NULL,FILE_BEGIN)){free(buffer);CloseHandle(hFile);return 0;}
	if(!ReadFile(hFile,buffer,sizeof(IMAGE_SECTION_HEADER)*nrSections,&nrBytesRead,NULL) || nrBytesRead!= sizeof(IMAGE_SECTION_HEADER)*nrSections){free(buffer);CloseHandle(hFile);return 0;}
	
	for(int i=0;i<nrSections;i++)
	{
		sect = &((IMAGE_SECTION_HEADER*)buffer)[i];
		if((sect->PointerToRawData+sect->SizeOfRawData) > overlayPos)
			overlayPos = sect->PointerToRawData+sect->SizeOfRawData;
	}

	free(buffer);CloseHandle(hFile);
	return overlayPos;
	
}
void CGMLInstallerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CGMLInstallerDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	RECT cRect;
	CDialogEx::OnPaint();
	

	GetClientRect(&cRect);
	int res = dc.SaveDC();

	dc.RestoreDC(res);
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGMLInstallerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

DWORD CGMLInstallerDlg::GetNecesaryBytesForFrameworkComponent()
{
	GString temp;
	char* fileName;
	DWORD necesaryBytes = 0;
	
	for (int tr=0;tr<gmlPackage.Header->TotalFiles;tr++)
	{
		fileName = gmlPackage.GetFileName(tr);
		temp.Set(fileName);
		if(!temp.Contains("sdk",true) && !temp.EndsWith(".py",true))
		{
			necesaryBytes+=gmlPackage.Header->Files[tr].UncompressedSize;
		}
	}
	return necesaryBytes;
}

DWORD CGMLInstallerDlg::GetNecesaryBytesForPythonComponent()
{
	GString temp;
	char* fileName;
	DWORD necesaryBytes = 0;
	
	for (int tr=0;tr<gmlPackage.Header->TotalFiles;tr++)
	{
		fileName = gmlPackage.GetFileName(tr);
		temp.Set(fileName);
		if(temp.EndsWith(".py",true))
		{
			necesaryBytes+=gmlPackage.Header->Files[tr].UncompressedSize;
		}
	}
	return necesaryBytes;
}
DWORD CGMLInstallerDlg::GetNecesaryBytesForGUIComponent()
{
	return 0;
}
DWORD CGMLInstallerDlg::GetNecesaryBytesForSDKComponent()
{
	GString temp;
	char* fileName;
	DWORD necesaryBytes = 0;
	
	for (int tr=0;tr<gmlPackage.Header->TotalFiles;tr++)
	{
		fileName = gmlPackage.GetFileName(tr);
		temp.Set(fileName);
		if(temp.Contains("sdk",true))
		{
			necesaryBytes+=gmlPackage.Header->Files[tr].UncompressedSize;
		}
	}
	return necesaryBytes;
}
DWORD CGMLInstallerDlg::GetNecesaryBytes()
{
	DWORD necesaryBytes;

	if(!IsWindow(listComponents.GetSafeHwnd()))
		return 0;

	necesaryBytes = 0;
	for(int i=0;i<listComponents.GetItemCount(); i++)
	{
		if(listComponents.GetCheck(i))
		{
			switch(i)
			{
			case COMPONENT_GML:
				necesaryBytes+=GetNecesaryBytesForFrameworkComponent();
				break;
			case COMPONENT_PYTHON:
				necesaryBytes+=GetNecesaryBytesForPythonComponent();
				break;
			case COMPONENT_GUI:
				necesaryBytes+=GetNecesaryBytesForGUIComponent();
				break;
			case COMPONENT_SDK:
				necesaryBytes+=GetNecesaryBytesForSDKComponent();
				break;
			}
		}
	}
	
	return necesaryBytes;
}
ULARGE_INTEGER CGMLInstallerDlg::CalculateFreeSpace(char* path,DWORD size)
{
	char testPath[4];

	ULARGE_INTEGER freeBytes;
	freeBytes.QuadPart = 0;
	if(size  <3)
		return freeBytes;
	strncpy(testPath,path,3);
	testPath[3] = '\0';
	if(testPath[1]!=':' && testPath[2]!='\\')
		return freeBytes;

	if(!GetDiskFreeSpaceEx(testPath,0,0,&freeBytes))
	{
		freeBytes.QuadPart = 0;
		return freeBytes;
	}
	return freeBytes;

}

CString CGMLInstallerDlg::GetDiskFreeSpaceStr(char* path, DWORD size)
{
	ULARGE_INTEGER freeSpace;
	CString unit;
	float DiskSize = 0;
	CString returnStr;
	freeSpace = CalculateFreeSpace(path,size);
	if(freeSpace.QuadPart != 0)
	{
		if(freeSpace.QuadPart / 1024 < 1024 && DiskSize == 0)
		{
			DiskSize = (float)freeSpace.QuadPart / (float)1024;
			unit = "Kb";
		}

		if(freeSpace.QuadPart / (1024*1024) < 1024 && DiskSize == 0)
		{
			DiskSize = (float)freeSpace.QuadPart / (float)(1024*1024);
			unit = "Mb";
		}

		if(freeSpace.QuadPart / (1024*1024*1024) < 1024 && DiskSize == 0)
		{
			DiskSize = (float)freeSpace.QuadPart / (float)(1024*1024*1024);
			unit = "Gb";
		}
		if(DiskSize == 0)
		{
			DiskSize = (double)freeSpace.QuadPart / (double)(1024.0*1024.0*1024.0*1024);
			unit = "TeraB";
		}
	}

	returnStr.Format("Space Available: %.2f%s",DiskSize,unit);
	return returnStr;
}
CString CGMLInstallerDlg::GetDiskNecesarySpaceStr()
{
	float size = 0;
	CString unit;
	CString returnStr;
	DWORD bytesNeeded = GetNecesaryBytes();
	if(bytesNeeded / 1024 < 1024)
	{
		size = (float)bytesNeeded / 1024;
		unit = "Kb";
	}

	if((bytesNeeded / (1024*1024) < 1024) && (size == 0))
	{
		size = (float)bytesNeeded / (1024*1024);
		unit = "Mb";
	}

	returnStr.Format("Space Required: %.2f%s",size,unit);

	return returnStr;
}
void CGMLInstallerDlg::AddComponentsTab()
{

	char defaultInstallPath[MAX_PATH];
	CString temp;

	
	staticComponents1.Create("Choose Components", WS_CHILD | WS_VISIBLE ,CRect(10,20,200,40),this);
	
	staticComponents1.SetFont(&boldFont);
	AddControl(&staticComponents1,TAB_COMPONENTS);

	staticComponents2.Create("Choose which features of GML you want to install.",WS_CHILD | WS_VISIBLE | SS_SIMPLE ,CRect(20,40,400,60),this);
	staticComponents2.SetFont(&simpleFont);
	AddControl(&staticComponents2,TAB_COMPONENTS);

	listComponents.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_LIST,CRect(20,80,310,300),this,ID_LIST_COMPONENTS);
	listComponents.SetExtendedStyle(LVS_EX_CHECKBOXES| LVS_EX_TRACKSELECT);
	AddControl(&listComponents,TAB_COMPONENTS);
	listComponents.InsertItem(COMPONENT_GML,"GML Framework",0);
	listComponents.SetCheck(COMPONENT_GML,true);
	intallComponents[COMPONENT_GML] = true;
	//listComponents.InsertItem(1,"GML Algorithms",0);
	//listComponents.InsertItem(2,"GML Connectors",0);
	//listComponents.InsertItem(3,"GML Notifiers",0);
	listComponents.InsertItem(COMPONENT_PYTHON,"GML Python Module",0);
	listComponents.InsertItem(COMPONENT_GUI,"GML Graphical Interface",0);
	listComponents.InsertItem(COMPONENT_SDK,"GML SDK",0);



	groupBoxDescription.Create("Description",WS_CHILD | WS_VISIBLE | BS_GROUPBOX,CRect(330,70,500,300),this,0);
	groupBoxDescription.SetFont(&simpleFont);
	
	AddControl(&groupBoxDescription,TAB_COMPONENTS);

	componentDescription.Create("Move the mouse over a component to see its description",WS_CHILD | WS_VISIBLE  | WS_GROUP,CRect(340,85,490,290),this);
	componentDescription.SetFont(&simpleFont);
	AddControl(&componentDescription,TAB_COMPONENTS);

	groupBoxDestinationFolder.Create("Destination Folder",WS_CHILD | WS_VISIBLE | BS_GROUPBOX, CRect(20,320,500,380),this,NULL);
	groupBoxDestinationFolder.SetFont(&simpleFont);
	AddControl(&groupBoxDestinationFolder,TAB_COMPONENTS);

	editDestinationFolder.CreateEx(WS_EX_CLIENTEDGE,"EDIT",NULL,WS_CHILD|WS_VISIBLE,CRect(40,345,400,365),this,ID_EDIT_DESTINATIONFOLDER);
	editDestinationFolder.SetFont(&simpleFont);
	
	
	staticSpaceRequired.Create("Space Required: Not determined",WS_CHILD | WS_VISIBLE,CRect(40,395,300,415),this,NULL);
	staticSpaceRequired.SetFont(&simpleFont);
	AddControl(&staticSpaceRequired,TAB_COMPONENTS);
	staticSpaceAvailable.Create("Space Available: Not determined",WS_CHILD | WS_VISIBLE,CRect(40,415,300,435),this,NULL);
	staticSpaceAvailable.SetFont(&simpleFont);
	AddControl(&staticSpaceAvailable,TAB_COMPONENTS);

	//seteaza necesary space
	CString necesaryBytes;
	necesaryBytes = GetDiskNecesarySpaceStr();
	staticSpaceRequired.SetWindowText(necesaryBytes);
	//
	
	if(SHGetSpecialFolderPath(NULL,defaultInstallPath,CSIDL_PROGRAM_FILES,false))
	{
		if(strlen(defaultInstallPath) <  254)
			strcat(defaultInstallPath,"\\GML");
		editDestinationFolder.SetWindowText(defaultInstallPath);

		temp = GetDiskFreeSpaceStr(defaultInstallPath,strlen(defaultInstallPath));
		staticSpaceAvailable.SetWindowText(temp);

	}

	
	
	AddControl(&editDestinationFolder,TAB_COMPONENTS);
	
	btnBrowse.Create("Browse ...",WS_CHILD | WS_VISIBLE,CRect(410,343,490,366),this, ID_BUTTON_BROWSE);
	btnBrowse.SetFont(&simpleFont);
	AddControl(&btnBrowse,TAB_COMPONENTS);


	
	btnNext.Create("Next >",WS_VISIBLE | WS_CHILD,CRect(365,498,445,520),this,ID_BUTTON_NEXT);
	AddControl(&btnNext,TAB_COMPONENTS);
	btnNext.SetFont(&simpleFont);


	btnBack.Create("< Back",WS_VISIBLE | WS_CHILD,CRect(277,498,357,520),this,ID_BUTTON_BACK);
	AddControl(&btnBack,TAB_COMPONENTS);
	btnBack.SetFont(&simpleFont);

	

}

void CGMLInstallerDlg::AddProgressTab()
{
	//editDestinationFolder.CreateEx(WS_EX_CLIENTEDGE,"EDIT",NULL,WS_CHILD|WS_VISIBLE,CRect(40,345,400,365),this,ID_EDIT_DESTINATIONFOLDER);
	DWORD totalBytes;
	GString temp;
	
	groupBoxProgress.Create("Installing GML",WS_CHILD | WS_VISIBLE | BS_GROUPBOX ,CRect(20,20,530,250),this,0);
	groupBoxProgress.SetFont(&simpleFont);
	AddControl(&groupBoxProgress,TAB_PROGRESS);
	
	progressBar.Create(WS_CHILD | WS_VISIBLE | PBS_SMOOTH | WS_GROUP,CRect(30,60,520,80),this,NULL);
	
	totalBytes = GetNecesaryBytes();
	progressBar.SetStep(1);
	progressBar.SetRange(1,totalBytes/0x0fff);
	
	AddControl(&progressBar,TAB_PROGRESS);
	
	staticProgressStatus.Create("Installing: not determined", WS_CHILD | WS_VISIBLE , CRect(30,100,400,120),this,NULL);
	staticProgressStatus.SetFont(&simpleFont);
	AddControl(&staticProgressStatus,TAB_PROGRESS);

	staticLinieProgress.Create("",WS_CHILD |WS_VISIBLE | SS_GRAYFRAME, CRect(30,130,300,131),this,NULL);
	AddControl(&staticLinieProgress,TAB_PROGRESS);
//GML framework info
	staticProgress1.Create("GML Framework:", WS_CHILD | WS_VISIBLE , CRect(30,140,150,160),this,NULL);
	staticProgress1.SetFont(&simpleFont);
	AddControl(&staticProgress1,TAB_PROGRESS);

	//valoare pentru GML Framework
	staticProgressGMLStatus.Create("Install not started", WS_CHILD | WS_VISIBLE , CRect(180,140,400,160),this,NULL);
	staticProgressGMLStatus.SetFont(&simpleFont);
	AddControl(&staticProgressGMLStatus,TAB_PROGRESS);

	globalStatusColor = RGB(0,0,0);
	globalStatus.Create("Install not started",WS_CHILD | WS_VISIBLE, CRect(220,350,400,390),this);
	//globalProgressFont.CreateFontA(14,0,0,0,FW_BOLD,FALSE,FALSE,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,TEXT("Arial"));
	AddControl(&globalStatus,TAB_PROGRESS);


	finishButton.Create("&Finish",WS_VISIBLE |WS_CHILD |BS_PUSHBUTTON,CRect(460,498,537,520),this,ID_BUTTON_FINISH);
	finishButton.SetFont(&simpleFont);
	AddControl(&finishButton,TAB_PROGRESS);
//GML Python info
	staticProgress2.Create("GML Pyhon Module:", WS_CHILD | WS_VISIBLE , CRect(30,160,150,180),this,NULL);
	staticProgress2.SetFont(&simpleFont);
	AddControl(&staticProgress2,TAB_PROGRESS);

	//valaorea pentru GML Python Module:
	staticProgressPythonStatus.Create("Install not started", WS_CHILD | WS_VISIBLE , CRect(180,160,400,180),this,NULL);
	staticProgressPythonStatus.SetFont(&simpleFont);
	AddControl(&staticProgressPythonStatus,TAB_PROGRESS);
//GML GUI info
	staticProgress3.Create("GML Graphical Interface:", WS_CHILD | WS_VISIBLE , CRect(30,180,150,200),this,NULL);
	staticProgress3.SetFont(&simpleFont);
	AddControl(&staticProgress3,TAB_PROGRESS);
	//valoarea pentru GML Graphical Interface
	staticProgressGUIStatus.Create("Install not started", WS_CHILD | WS_VISIBLE , CRect(180,180,400,200),this,NULL);
	staticProgressGUIStatus.SetFont(&simpleFont);
	AddControl(&staticProgressGUIStatus,TAB_PROGRESS);

//SDK info
	staticProgress4.Create("GML SDK:", WS_CHILD | WS_VISIBLE , CRect(30,200,150,220),this,NULL);
	staticProgress4.SetFont(&simpleFont);
	AddControl(&staticProgress4,TAB_PROGRESS);
	//valoarea pentru GML SDK
	staticProgressSDKStatus.Create("Install not started", WS_CHILD | WS_VISIBLE , CRect(180,200,400,220),this,NULL);
	staticProgressSDKStatus.SetFont(&simpleFont);
	AddControl(&staticProgressSDKStatus,TAB_PROGRESS);
	
		//staticProgressGUIStatus;

}
void CGMLInstallerDlg::OnBnClickedPanel1BtnCancel()
{
	OnCancel();
}

void CGMLInstallerDlg::OnCancel()
{
	if(currentTab == TAB_PROGRESS)
	{
		this->EndDialog(0);
		return;
	}
	if(IDYES == MessageBox("Are you sure you want to quit GML installer?","GML installer",MB_ICONEXCLAMATION | MB_YESNO))
	{
		this->EndDialog(-1);
	}
}


unsigned char CGMLInstallerDlg::GetPackage(char* fileName)
{
	GString temp;
	temp.Set(fileName);
	
	if(temp.EndsWith(".py",true))
	{
		return COMPONENT_PYTHON;
	}
	if(temp.Contains("sdk",true))
	{
		return COMPONENT_SDK;
	}

	return COMPONENT_GML;

}
void CGMLInstallerDlg::ShowControls(unsigned char newTab)
{
	if(currentTab >= NRTABS)
		return;
	currentTab = newTab;

	for(int i =0;i< nrControls;i++)
	{
		if(controls[i].tabId != currentTab && controls[i].tabId != TAB_SHOWALWAYS)
			controls[i].wnd->ShowWindow(SW_HIDE);
		else
			controls[i].wnd->ShowWindow(SW_SHOW);
	}
	this->RedrawWindow();
}
bool CGMLInstallerDlg::AddControl(CWnd* newWindow, unsigned char tabId)
{
	if(tabId >= NRTABS)
		return false;
	if(nrControls >=  MAX_CONTROL_NR)
		return false;
	controls[nrControls].tabId = tabId;
	controls[nrControls++].wnd = newWindow;
	return true;
}
void CGMLInstallerDlg::OnComponentsListViewHoover(NMHDR* pNMHDR, LRESULT* pResult)
{

	NM_LISTVIEW* item = (NM_LISTVIEW*)pNMHDR;
	if(item->iItem >=0 && item->iItem < MAX_COMPONENTS_NR)
		componentDescription.SetWindowText(descriptions[item->iItem]);

	CString necesaryBytes;
	necesaryBytes = GetDiskNecesarySpaceStr();
	staticSpaceRequired.SetWindowText(necesaryBytes);
}

void CGMLInstallerDlg::OncomponentListViewChanged(NMHDR* lparam, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)lparam;
	int ns = pNMListView->uNewState & LVIS_STATEIMAGEMASK; 
	
	
	if(pNMListView->iItem == COMPONENT_GML)
	{
		if ((ns & 0x2000) == 0)
		{
			*pResult = TRUE;
		}
		else
		{
			*pResult = FALSE;
		}
	}
	else
	{
		intallComponents[pNMListView->iItem] = ((ns & 0x2000) == 0x2000);
	}

	
}
void CGMLInstallerDlg::OnBnClickedPanel1BtnAgree()
{
	ShowControls(TAB_COMPONENTS);
}

bool CGMLInstallerDlg::GetRegistryPythonPaths(GString* pythonPaths, unsigned char* nrElements, unsigned int registryType) //KEY_WOW64_32KEY sau KEY_WOW64_64KEY
{
	HKEY softwareKey,installPathKey;
	DWORD nrSubKeys;
	DWORD keyNameSize;
	char keyName[20];
	char pythonPath[MAX_PATH];
	DWORD size;
	GString temp;
	if(registryType != KEY_WOW64_32KEY && registryType != KEY_WOW64_64KEY)
		return false;
	//HKLM\Software\Python\PythonCore\2.6\InstallPath  --> (Default)
	if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,"Software\\Python\\PythonCore",NULL,KEY_ENUMERATE_SUB_KEYS  | registryType |  KEY_QUERY_VALUE ,&softwareKey)) return false;
	if(ERROR_SUCCESS != RegQueryInfoKey(softwareKey,NULL,NULL,NULL,&nrSubKeys,NULL,NULL,NULL,NULL,NULL,NULL,NULL)) { RegCloseKey(softwareKey); return false;}
	for(int i=0;i<nrSubKeys;i++)
	{
		keyNameSize = 20;
		RegEnumKeyEx(softwareKey,i,keyName,&keyNameSize,NULL,NULL,NULL,NULL);
		//sunt in HKLM\Software\Python\PythonCore
		temp.SetFormated("%s\\InstallPath",keyName);
		if(ERROR_SUCCESS != RegOpenKeyEx(softwareKey,temp.GetText(),NULL,KEY_QUERY_VALUE,&installPathKey)) { RegCloseKey(softwareKey); return false;}
		size = MAX_PATH;
		if(ERROR_SUCCESS != RegQueryValueEx(installPathKey,"",NULL,NULL,(BYTE*)pythonPath,&size)){RegCloseKey(installPathKey);RegCloseKey(softwareKey); return false;}
		if(*nrElements >= MAX_PYTHON_PATHS){RegCloseKey(installPathKey);RegCloseKey(softwareKey); return false;}
		
		pythonPaths[*nrElements].Set(pythonPath);
		*nrElements=(*nrElements) + 1;
		RegCloseKey(installPathKey);		
		//
	}
	RegCloseKey(softwareKey);
	return true;
}

bool CGMLInstallerDlg::InstallPythonModule(char* installPath)
{
	GString temp;
	GString pythonFile;
	GString fileName;
	HANDLE hFile;
	DWORD nrBytesWritten;
	

	GString pythonPaths[MAX_PYTHON_PATHS];
	unsigned char nrPythonPaths = 0;
	
	for (int tr=0;tr<gmlPackage.Header->TotalFiles;tr++)
	{
		temp.Set(gmlPackage.GetFileName(tr));
		if(temp.EndsWith(".py"))
		{
			//set gml path in gml.py
			fileName.Set(installPath);
			fileName.PathJoinName(gmlPackage.GetFileName(tr));
			if(!pythonFile.LoadFromFile(fileName)) return false;
			temp.SetFormated("GML_PATH = r\"%s\"",installPath);
			pythonFile.Replace("GML_PATH = r\"E:\\lucru\\GML\\gml\\prj\\gml\\Release\"",temp,true);
			hFile = CreateFile(fileName.GetText(),GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
			if( hFile == INVALID_HANDLE_VALUE)
				return false;
			if(!WriteFile(hFile,pythonFile.GetText(),pythonFile.GetSize(),&nrBytesWritten,NULL) || nrBytesWritten !=pythonFile.GetSize())
			{
				CloseHandle(hFile);
				return false;
			}
			CloseHandle(hFile);
			
			//copie fisierul in fiecare python
			if(!GetRegistryPythonPaths(pythonPaths,&nrPythonPaths,KEY_WOW64_32KEY))
				return false;
			if(!GetRegistryPythonPaths(pythonPaths,&nrPythonPaths,KEY_WOW64_64KEY))
				return false;
			//copie fisierul in pathul python
			for(int i=0;i< nrPythonPaths;i++)
			{
				temp.SetFormated(pythonPaths[i]);
				temp.PathJoinName("lib");
				temp.PathJoinName(gmlPackage.GetFileName(tr));

				if(!CopyFile(fileName.GetText(),temp.GetText(),false))
					return false;
			}
	
			DeleteFile(fileName);
		}
	}

	

	return true;
}

bool CGMLInstallerDlg::SetInstallComponentStatus(unsigned char component, char* str)
{	
	CStatic* element;
	switch(component)
	{
	case COMPONENT_GML:
		element = &staticProgressGMLStatus;
		break;
	case COMPONENT_PYTHON:
		element = &staticProgressPythonStatus;
		break;
	case COMPONENT_GUI:
		element = &staticProgressGUIStatus;
		break;

	case COMPONENT_SDK:
		element = &staticProgressSDKStatus;
		break;
	}

	element->SetWindowTextA(str);
	this->RedrawWindow();
	return true;
}

bool CGMLInstallerDlg::AddGmlToSystemPath(char* installPath)
{
	//setarile se afla aici: HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Environment
	char* pathVar;
	DWORD pathVarSize = 0;
	HKEY environmentKey;
	GString temp;


	if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment",NULL,KEY_WRITE | KEY_READ,&environmentKey) ){return false;}
	if(ERROR_SUCCESS != RegQueryValueEx(environmentKey,"Path",NULL,NULL,NULL,&pathVarSize)){RegCloseKey(environmentKey); return false;}
	pathVarSize+=strlen(installPath)+2;
	pathVar = (char*)malloc(pathVarSize); //; si \0
	if(pathVar == NULL){RegCloseKey(environmentKey); return false;}

	if(ERROR_SUCCESS != RegQueryValueEx(environmentKey,"Path",NULL,NULL,(BYTE*)pathVar,&pathVarSize)){RegCloseKey(environmentKey); return false;}
	//daca este, nu-l mai adaug odata
	temp.Set(pathVar);
	if(temp.Find(installPath,true) > 0)
	{
		//inseamna ca exista
		RegCloseKey(environmentKey);
		return true;
	}
	temp.SetFormated("%s;%s",pathVar,installPath);
	if(ERROR_SUCCESS!=RegSetValueEx(environmentKey,"Path",NULL,REG_EXPAND_SZ,(BYTE*)temp.GetText(),temp.GetSize())){RegCloseKey(environmentKey); return false;}

	RegCloseKey(environmentKey);

	return  (SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE,0,(LPARAM)(_T("Environment")),SMTO_BLOCK | SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG, 5000, NULL )!=0);
	//return true;
}

bool CGMLInstallerDlg::AddInstallKeyForGML(char* installPath)
{
	HKEY software,gmlKey;
	GString temp;

	temp.Set(installPath);

	if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,"Software",NULL,KEY_WRITE | KEY_READ,&software) ){return false;}
	if(ERROR_SUCCESS != RegCreateKeyEx(software,"GML",NULL,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&gmlKey,NULL)){RegCloseKey(software); return false;}
	if(ERROR_SUCCESS !=RegSetValueEx(gmlKey,"InstallPath",NULL,REG_SZ,(BYTE*)temp.GetText(),temp.GetSize())){RegCloseKey(gmlKey); RegCloseKey(software); return false;}
	RegCloseKey(gmlKey); RegCloseKey(software); 
	return true;
}

void CGMLInstallerDlg::ShowErrorAndUpdateGlobalStatus(char* mbError)
{
	globalStatusColor = RGB(255,0,0);
	globalStatus.SetWindowText("Install Failed");
	finishButton.EnableWindow(TRUE);
	MessageBox(mbError,"Install Error",MB_OK | MB_ICONERROR);
}
void CGMLInstallerDlg::OnBnClickedNext()
{
	int low,up;
	unsigned char component;
	
	ShowControls(++currentTab);

	//instaleaza si calculeaza progress barul
	char installPath[MAX_PATH];
	GString destinationPath,temp;
	GetDlgItem(BTN_CANCEL)->ShowWindow(SW_HIDE);
	finishButton.EnableWindow(FALSE);
	
	editDestinationFolder.GetWindowText(installPath,MAX_PATH);
	
	for(int i=0;i<MAX_COMPONENTS_NR;i++)
	{
		if(!intallComponents[i])
			SetInstallComponentStatus(i,"Not selected for install");
	}
	

	progressBar.GetRange(low,up);
	globalStatus.SetWindowText("Copying files");
	for (int tr=0;tr<gmlPackage.Header->TotalFiles;tr++)
	{
		//printf("[CRC:%08X] [Compressed:%8d] [Uncompressed:%8d] => %s\n",drp.Header->Files[tr].CRC32,drp.Header->Files[tr].CompressedSize,drp.Header->Files[tr].UncompressedSize,drp.GetFileName(tr));
	
		destinationPath.Set(installPath);
		destinationPath.PathJoinName(gmlPackage.GetFileName(tr));
		if (gmlPackage.Extract(tr,destinationPath.GetText())==false)
		{
			
			ShowErrorAndUpdateGlobalStatus(gmlPackage.GetError());	
			return;
		}
		temp.SetFormated("Copying: %s",gmlPackage.GetFileName(tr));
		groupBoxProgress.ShowWindow(SW_SHOW);
		component = GetPackage(gmlPackage.GetFileName(tr));
		if(intallComponents[component])
		{
			SetInstallComponentStatus(component,"Copying files...");
			staticProgressStatus.SetWindowText(temp);
			for(int j=0;j<gmlPackage.Header->Files[tr].UncompressedSize / 0x0fff;j++)
			{
				if(progressBar.GetPos() != up)
					progressBar.StepIt();
			}
		}
		
	}
	
	
	for(int i=0;i<MAX_COMPONENTS_NR;i++)
	{
		if(intallComponents[i])
			SetInstallComponentStatus(i,"Installed");

	}

	globalStatus.SetWindowText("Installing registry");
	this->RedrawWindow();
	
	if(!AddGmlToSystemPath(installPath))
	{
		SetInstallComponentStatus(COMPONENT_GML,"Failed to add GML to system path");
		ShowErrorAndUpdateGlobalStatus("Failed to add GML to system path");	
		return;
	}
	
	if(!AddInstallKeyForGML(installPath))
	{
		SetInstallComponentStatus(COMPONENT_GML,"Failed to create GML install key");	
		ShowErrorAndUpdateGlobalStatus("Failed to add GML to system registry");	
		return;
	}


	

	if(intallComponents[COMPONENT_PYTHON])
	{
		//modifica pahtul din GML py
		SetInstallComponentStatus(COMPONENT_PYTHON,"Installing ...");
		if(!InstallPythonModule(installPath))
		{
			SetInstallComponentStatus(COMPONENT_PYTHON,"Failed");
			ShowErrorAndUpdateGlobalStatus("Failed to add insatll python module");	
			return;
		}
		
		SetInstallComponentStatus(COMPONENT_PYTHON,"Installed");
	}


	finishButton.EnableWindow(TRUE);
	progressBar.SetPos(up);
	staticProgressStatus.SetWindowTextA("Files copied");
	globalStatusColor = RGB(0,255,0);
	globalStatus.SetWindowText("Install Completed");
	

}

void CGMLInstallerDlg::OnBnClickedBack()
{
	ShowControls(--currentTab);
}


void CGMLInstallerDlg::OnBnClickedBrowse()
{
	CString destinationPath;
	//editDestinationFolder.GetWindowText(destinationPath);
	BROWSEINFO bi;
	
	::ZeroMemory(&bi, sizeof(bi));
	TCHAR	buffer[MAX_PATH];

	bi.hwndOwner = this->m_hWnd;
	bi.lpszTitle = _T("Choose GML path");
	bi.pidlRoot = 0;
	bi.ulFlags = BIF_NEWDIALOGSTYLE;

	

	LPITEMIDLIST pItem = ::SHBrowseForFolder(&bi);
	if (pItem != NULL)
	{

		::SHGetPathFromIDList(pItem, buffer);
		destinationPath = buffer;
		if(destinationPath.Right(1).Compare("\\") == 0)
			destinationPath.Left(destinationPath.GetLength() - 1);
		if(destinationPath.Right(4).CompareNoCase("GML") == 0)
			editDestinationFolder.SetWindowText(buffer);
		else
		{
			destinationPath+="\\GML";
			editDestinationFolder.SetWindowText(destinationPath);
		}
			
	}
}

//schimba culoarea la default installer


HBRUSH CGMLInstallerDlg::OnCtlColor(CDC* pDC, CWnd *pWnd, UINT nCtlColor)
{

	if(pWnd == &globalStatus)
	{
		pDC->SetTextColor(globalStatusColor);
		pDC->SetBkMode(TRANSPARENT);

		return (HBRUSH)GetSysColorBrush(COLOR_3DFACE);
	}
	
    return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}