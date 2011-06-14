#pragma once
#include "bitem.h"
class BHeader :
	public BItem
{
public:
	BHeader(void);
	~BHeader(void);

	bool Create(char* strLabel,unsigned char offset,CWnd* parent, UINT txtID);
	void OnClick(UINT nFlags,CPoint point);
	void Paint();

};

