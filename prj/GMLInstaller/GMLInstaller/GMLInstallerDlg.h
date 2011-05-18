
// GMLInstallerDlg.h : header file
//

#pragma once

#include "GDropper.h"

#define MAX_PYTHON_PATHS	10

#define MAX_CONTROL_NR		100
#define MAX_COMPONENTS_NR	4
#define TAB_SHOWALWAYS	0		
#define TAB_EULA		1
#define TAB_COMPONENTS	2
#define TAB_PROGRESS	3
#define NRTABS			4


#define ID_LIST_COMPONENTS			0x1000
#define ID_EDIT_DESTINATIONFOLDER	0x1001
#define ID_BUTTON_BROWSE			0x1002
#define ID_BUTTON_NEXT				0x1003
#define ID_BUTTON_BACK				0x1004
// CGMLInstallerDlg dialog


#define COMPONENT_GML		0
#define COMPONENT_PYTHON	1
#define COMPONENT_GUI		2
#define COMPONENT_SDK		3

static char* descriptions[MAX_COMPONENTS_NR] =
{
	"The Main Component. Used to create and run algorithms, connect to notifiers (for example: graphic notifier), connect to databases, etc.",
	//"Many machine learning algorithms. Included are BatchPercetron, AdelainePerceptron, NaiveBaias,RosenblattPerceptron,etc.",
	//"These components allow the algorithm to connect to diferent data sources, such as files or databases",
	//"Using these components, one can see data sent by the framework in diferent ways. For example, it can be displayed on a console or in a graphical interface",
	"By using thise module, one can create and test algrorithms using python. Thus, one can use the advantages of diferent structures in a scripting language, such as dictionaries",
	"Allows an user to save and load configurations using a graphical interface. The results are also displayed in a more organised manner then are on a console interface",
	"Using the SDK you can create other algorithms, notifiers, connectors than those already provided by the framework"
};

class CGMLInstallerDlg : public CDialogEx
{
	struct ControlInfo
	{
		unsigned char tabId;
		CWnd* wnd;
	};
	
// Construction
public:
	CGMLInstallerDlg(CWnd* pParent = NULL);	// standard constructor
	ControlInfo controls[MAX_CONTROL_NR];
	DWORD nrControls;
	DWORD currentTab;

// Dialog Data
	enum { IDD = IDD_GMLINSTALLER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void ShowControls(unsigned char newTab);
	bool AddControl(CWnd* newWindow, unsigned char tabId);
	void AddComponentsTab();
	void AddProgressTab();
	ULARGE_INTEGER CalculateFreeSpace(char* path,DWORD size);
	CString GetDiskFreeSpaceStr(char* path, DWORD size);
	CString GetDiskNecesarySpaceStr();
	DWORD GetNecesaryBytesForFrameworkComponent();
	DWORD GetNecesaryBytesForPythonComponent();
	DWORD GetNecesaryBytesForGUIComponent();
	DWORD GetNecesaryBytesForSDKComponent();
	bool AddGmlToSystemPath(char* installPath);
	bool InstallPythonModule(char* installPath);
	bool SetInstallComponentStatus(unsigned char component, char* str);
	bool AddInstallKeyForGML(char* installPath);
	unsigned char GetPackage(char* fileName);
	
	DWORD GetNecesaryBytes();
	bool intallComponents[NRTABS];

	//Components Tab:
	CFont boldFont;
	CFont simpleFont;
	CBrush greyDeschis;
	CStatic staticComponents1;
	CStatic staticComponents2;
	CStatic staticComponents3;
	CStatic componentDescription;
	CButton groupBoxDescription;
	CButton groupBoxDestinationFolder;
	CEdit	editDestinationFolder;
	CButton btnBrowse;
	CListCtrl listComponents;
	CStatic staticSpaceAvailable;
	CStatic staticSpaceRequired;
	
	CButton btnNext;
	CButton btnBack;

	//Progress Tab:
	CButton groupBoxProgress;
	CProgressCtrl progressBar;
	CStatic staticProgressStatus;
	CStatic staticLinieProgress;
	CStatic staticProgressGMLStatus;
	CStatic staticProgressPythonStatus;
	CStatic staticProgressSDKStatus;
	CStatic staticProgressGUIStatus;

	CStatic staticProgress1,staticProgress2,staticProgress3,staticProgress4;

	
	
	GDropper gmlPackage;


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnComponentsListViewHoover(NMHDR* /*pNMHDR*/, LRESULT* pResult);
	afx_msg void OncomponentListViewChanged(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedPanel1BtnCancel();
	afx_msg void OnBnClickedPanel1BtnAgree();
	bool GetRegistryPythonPaths(GString* pythonPaths, unsigned char* nrElements,unsigned int registryType);
	


	afx_msg void OnBnClickedNext();
	afx_msg void OnBnClickedBack();
	afx_msg void OnBnClickedBrowse();
	afx_msg void GetDefaultInstallPath(char* path,unsigned int size);
};
