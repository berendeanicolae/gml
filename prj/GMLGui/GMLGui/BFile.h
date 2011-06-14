#pragma once

#include "BItem.h"
#define BUTTON_BROWSE_WIDTH	15
#define BUTTON_BROWSE_HEIGHT	17
#define SPACE_BEFORE_BROWSE_BUTTON 3
#define BUTTON_BROWSE_ID	0x5000

class BFile: public BItem
{
public:
	BFile(void);
	~BFile(void);

	CEdit filePath;
	CButton btnBrowse;
	bool isFolder;

	
	bool Create(char* strLabel,unsigned char offset,CWnd* parent, UINT ID,bool isFolder = false);
	void	UpdatePosition();
	bool	SetText(char* txt);
	void	GetText(CString& str);

	afx_msg void OnBnClickedBrowse();
	DECLARE_MESSAGE_MAP()



};

