
// IAFrameworkDlg.cpp : implementation file
//

//pentru memory leakuri
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


#include "stdafx.h"
#include "IAFramework.h"
#include "IAFrameworkDlg.h"
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


// CIAFrameworkDlg dialog




CIAFrameworkDlg::CIAFrameworkDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CIAFrameworkDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	graphicResultWindow = NULL;
	currentAlgorithm = NULL;
}

void CIAFrameworkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CMB_ALGORITHMS, cmbAlgorithms);
}

BEGIN_MESSAGE_MAP(CIAFrameworkDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_SELCHANGE(IDC_CMB_ALGORITHMS, &CIAFrameworkDlg::OnCbnSelchangeCmbAlgorithms)
	ON_COMMAND(ID_FILE_SAVECONFIGURATION, &CIAFrameworkDlg::OnFileSaveconfiguration)
	ON_COMMAND(ID_FILE_OPENCONFIGURATION, &CIAFrameworkDlg::OnFileOpenconfiguration)
	ON_MESSAGE(WM_ALGORITHM_CONTAINER_CLOSED,&CIAFrameworkDlg::OnAlgorithmContainerClosed)
	
	ON_COMMAND(ID_FILE_RUN, &CIAFrameworkDlg::OnFileRun)
END_MESSAGE_MAP()


// CIAFrameworkDlg message handlers

BOOL CIAFrameworkDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	//pentru memory leakuri
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

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

	//creez obiectul

	if(!FindAlgorithms())
	{
		MessageBox(_T("There was an error getting algorithms"),_T("Error Retreaving Algorithms"),MB_OK | MB_ICONERROR);
		ExitProcess(-1);
	}

	if(cmbAlgorithms.GetCount() <=0)
	{
		MessageBox(_T("Could not find any algorithms in the Algorithms directory."),_T("Error finding algorithms"),MB_OK | MB_ICONERROR);
		ExitProcess(-1);
	}

	
	//currentAlgorithm = new AlgorithmObject("BatchPerceptron");
	m_PropCtrl.Create( this , IDC_STATIC_PROPCONTROL );
	//m_PropCtrl.SetPropertyHost(currentAlgorithm);
	m_PropCtrl.EnableCommentPane(true);
	cmbAlgorithms.SetCurSel(0);
	OnCbnSelchangeCmbAlgorithms();


	return TRUE;  // return TRUE  unless you set the focus to a control
}



void CIAFrameworkDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CIAFrameworkDlg::OnPaint()
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
HCURSOR CIAFrameworkDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

 

bool CIAFrameworkDlg::FindAlgorithms()
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




	//afla pathul curent

}

void CIAFrameworkDlg::DrawSelectedAlgorithm(char* configurationFile)
{
	unsigned int curSel;
	CString itemText;
	AlgorithmObject* oldAlgorithmObject;
	
	curSel = cmbAlgorithms.GetCurSel();
	if(curSel != CB_ERR)
	{
		cmbAlgorithms.GetLBText(curSel,itemText);
		oldAlgorithmObject = currentAlgorithm;
		currentAlgorithm = new AlgorithmObject((char*)itemText.GetString());
		//ca sa ma ajute sa misc proprietatile intre ele
		currentAlgorithm->SetPropertyControl(&m_PropCtrl);

		
		if(configurationFile!=NULL)
		{
			currentAlgorithm->SetConfigurationFile(configurationFile);
		}
		//asta il face sa se deseneze	
		m_PropCtrl.SetPropertyHost(currentAlgorithm);

	
		if(oldAlgorithmObject!= NULL)
		{
			delete(oldAlgorithmObject);
		}
	
	}
}

void CIAFrameworkDlg::OnCbnSelchangeCmbAlgorithms()
{
	DrawSelectedAlgorithm();
}


void CIAFrameworkDlg::OnFileSaveconfiguration()
{
	//currentAlgorithm->SaveCurrentConfiguration(//some file)
	if(currentAlgorithm == NULL)
		return;

	CString fileName;
	this->SetFocus();

	fileName = currentAlgorithm->algorithmName;
	CFileDialog dlg(FALSE/*Open=TRUE Save=False*/,
					"ia"/*Filename Extension*/,
					fileName/*Initial Filename*/,
					OFN_ENABLESIZING|OFN_EXPLORER/*Flags*/,
					"Invatare Automata(*.ia)|"/*Filetype Filter*/,
					this/*parent Window*/);
	if (dlg.DoModal() == IDOK)
	{
		fileName = dlg.GetPathName();
		if(!currentAlgorithm->SaveCurrentConfiguration((char*)fileName.GetString()))
		{
			AfxMessageBox("Sorry, but the configuration could not be saved");
			return;
		}
		AfxMessageBox("The configuration was saved saved successfully!",MB_OK | MB_ICONINFORMATION);

	}
	
}


void CIAFrameworkDlg::OnFileOpenconfiguration()
{
	GML::Utils::AttributeList atributeList;
	GML::Utils::Attribute* algName;
	char* algorithmName;
	CString fileName = "D:\\Shared VM\\share-write\\temp\\testGdt\\test2\\gml\\template2.txt";
	CString someAlgorithm;
	
	if(currentAlgorithm == NULL)
		return;

	fileName = currentAlgorithm->algorithmName;
	CFileDialog dlg(true/*Open=TRUE Save=False*/,
		"ia"/*Filename Extension*/,
		fileName/*Initial Filename*/,
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
			if(someAlgorithm.CollateNoCase(algorithmName)==0)
			{
				//am gasit algoritmul
				cmbAlgorithms.SetCurSel(i);
				DrawSelectedAlgorithm((char*)fileName.GetString());
				return;
			}
		}

		AfxMessageBox("Failed to find specified algorithm in current loaded algorithms.",MB_OK | MB_ICONERROR);
		return ;

	}


}

bool CIAFrameworkDlg::AlgorithmRun(GML::Utils::AttributeList*	attrList)
{
	GML::Algorithm::IAlgorithm	*alg;
	CString temp;
	
	if ((alg=GML::Builder::CreateAlgorithm(currentAlgorithm->algorithmName))==NULL)
	{
		temp.Format("[ERROR] -> Error creating '%s'",currentAlgorithm->algorithmName);
		AfxMessageBox(temp);
		return false;
	}
	if (alg->SetProperty(*attrList)==false)
	{
		temp.Format("[ERROR] -> Error updateing configuration to '%s'",currentAlgorithm->algorithmName);
		AfxMessageBox(temp);
		return false;
	}
	if (alg->Init()==false)
	{
		temp.Format("[ERROR] -> Error on initialization of '%s'",currentAlgorithm->algorithmName);
		AfxMessageBox(temp);
		return false;
	}
	if (alg->Execute()==false)
	{
		temp.Format("[ERROR] -> Error on execution of '%s'",currentAlgorithm->algorithmName);
		AfxMessageBox(temp);
		return false;
	}
	//alg->Wait();
	//printf("Algorithm '%s' done !\n",currentAlgorithm->algorithmName);
	return true;
}
bool CIAFrameworkDlg::StartGmlExec(GML::Utils::AttributeList*	attrList)
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
	if(!currentAlgorithm->SaveCurrentConfiguration(tempFileName,attrList))
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

void CIAFrameworkDlg::OnFileRun()
{

	GML::Utils::AttributeList	attrList;
	GML::Utils::Attribute* notifier;



	
	if(!currentAlgorithm->SaveConfigurationToAttributeList(&attrList))
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
		StartGmlExec(&attrList);
	}
	else if (strnicmp((char*)notifier->Data,"WrapperNotifier",15)==0)
	{
		if(graphicResultWindow == NULL)
		{
			graphicResultWindow = new GraphicNotifierContainer();
			graphicResultWindow->Create("Graphic Notifier",20,20,520,520,this,ID_ALGORITHM_CONTAINER);
		}	

		graphicResultWindow->AddAlgorithm(currentAlgorithm->algorithmName);
		AlgorithmRun(&attrList);
	}


}



void CIAFrameworkDlg::OnOK()
{
	if(currentAlgorithm!= NULL)
	{
		delete(currentAlgorithm);
	}
	CDialogEx::OnOK();
	
}



LRESULT CIAFrameworkDlg::OnAlgorithmContainerClosed(WPARAM p1, LPARAM p2)
{
	if(graphicResultWindow != NULL)
	{
		delete (graphicResultWindow);
		graphicResultWindow = NULL;
	}
	return 0;
}
