#pragma once

#include "BContainerSizes.h"
#define ITEM_NO_COLLAPSE	0
#define ITEM_COLLAPSED		1
#define ITEM_EXPANDED		2

class BItem : public CWnd
{
public:

	CFont	simpleFont;
	CFont	smallFont;
	unsigned char offset;
	char	collapsed;	//0 nu e prezent,
						//1 e collapsed
						//2 e expanded
	char	label[100];
	char*	description;
	char*	metadata;
	unsigned char elementType;
	DWORD	LabelWidth;


	bool Create(char* strLabel,unsigned char offset,DWORD Width,DWORD LabelWidth,CWnd* parent);
	void SetCollapsed(char state);
	DWORD GetHeight();
	
	DWORD			MoveElement(DWORD newYPos);
	void			SetWidth(DWORD width);
	void			SetLabelWidth(DWORD labelWidth);
	virtual void	UpdatePosition();
	bool			IsVisible();
	bool			SetDescription(char* des);
	bool			SetMetaData(char* metaData);
	char*			GetMetaData();
	char GetCollapsedState();

	BItem(void);
	~BItem(void);

	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags,CPoint point);
	virtual void Paint();
	virtual void OnClick(UINT nFlags,CPoint point);
	DECLARE_MESSAGE_MAP();
};

