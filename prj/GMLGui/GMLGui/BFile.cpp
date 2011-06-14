#include "StdAfx.h"
#include "BFile.h"


BFile::BFile(void)
{
	isFolder = false;
}


BFile::~BFile(void)
{
}


BEGIN_MESSAGE_MAP(BFile, BItem)

ON_BN_CLICKED(BUTTON_BROWSE_ID, OnBnClickedBrowse)
END_MESSAGE_MAP()


bool BFile::Create(char* strLabel,unsigned char offset,CWnd* parent, UINT ID,bool isFolder)
{
	BItem::Create(strLabel,offset,ELEMENT_WIDTH,LABEL_WIDTH,parent);

	CRect tempRect;
	GetClientRect(&tempRect);
	tempRect.left = tempRect.left+this->LabelWidth+SPACE_BETWEEN_ELEMENT;
	tempRect.right-=(SPACE_BEFORE_BROWSE_BUTTON + BUTTON_BROWSE_WIDTH);
	

	if(!filePath.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL  , tempRect,this,ID)){return false;};
	//tempRect.right+=SPACE_BETWEEN_ELEMENT;
	tempRect.left = tempRect.Width()+1;
	tempRect.right = tempRect.left+BUTTON_BROWSE_WIDTH;
	tempRect.top = tempRect.bottom-BUTTON_BROWSE_HEIGHT;
	if(!btnBrowse.Create("...",WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_NOTIFY  |BS_CENTER , tempRect,this,BUTTON_BROWSE_ID)){return false;};
	
	filePath.SetFont(&simpleFont,true);
	btnBrowse.SetFont(&simpleFont);
	this->isFolder = isFolder;
	UpdatePosition();

	return true;

}



void BFile::UpdatePosition()
{
	CRect tempRect;

	GetClientRect(&tempRect);
	tempRect.left = this->LabelWidth +  SPACE_BETWEEN_ELEMENT;
	tempRect.right -=(SPACE_BEFORE_BROWSE_BUTTON + BUTTON_BROWSE_WIDTH);
	filePath.MoveWindow(tempRect);

	tempRect.left = tempRect.right+1;
	tempRect.right = tempRect.left+BUTTON_BROWSE_WIDTH;
	tempRect.top = tempRect.bottom-BUTTON_BROWSE_HEIGHT;
	btnBrowse.MoveWindow(tempRect);
	
}


void BFile::OnBnClickedBrowse()
{
	if(isFolder)
	{
		CString destinationPath;
		//editDestinationFolder.GetWindowText(destinationPath);
		BROWSEINFO bi;
	
		::ZeroMemory(&bi, sizeof(bi));
		TCHAR	buffer[MAX_PATH];

		bi.hwndOwner = this->m_hWnd;
		bi.lpszTitle = _T("Choose path");
		bi.pidlRoot = 0;
		bi.ulFlags = BIF_NEWDIALOGSTYLE;

	

		LPITEMIDLIST pItem = ::SHBrowseForFolder(&bi);
		if (pItem != NULL)
		{

			::SHGetPathFromIDList(pItem, buffer);
			destinationPath = buffer;
			if(destinationPath.Right(1).Compare("\\") == 0)
				destinationPath.Left(destinationPath.GetLength() - 1);
			destinationPath+="\\*.*";
			filePath.SetWindowTextA(destinationPath);
			
		}

		return;
	}
	
	CFileDialog dlg(true/*Open=TRUE Save=False*/,
		"*.*"/*Filename Extension*/,
		NULL/*Initial Filename*/,
		OFN_ENABLESIZING|OFN_EXPLORER|OFN_FILEMUSTEXIST/*Flags*/,
		"All file types(*.*)|"/*Filetype Filter*/,
		this/*parent Window*/);
	if (dlg.DoModal() == IDOK)
	{
		
		filePath.SetWindowTextA(dlg.GetPathName());
	}

}
