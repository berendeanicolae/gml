#include "StdAfx.h"
#include "BContainer.h"


BEGIN_MESSAGE_MAP(BContainer, CWnd)
	//{{AFX_MSG_MAP(EPropListCtrl)
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BContainer::BContainer(void)
{
	vElements.Create(32, sizeof(BItem*));
	lastYpos = 0;
	lastId = 0;
}

bool BContainer::Create(unsigned int x, unsigned int y, unsigned int width, unsigned height,CWnd* parent,unsigned int ID)
{
	
	
	if(parent == NULL)	return false;
	if(!CWnd::Create(NULL,NULL,WS_CHILD | WS_VISIBLE | WS_VSCROLL, CRect(x,y,x+width, y+height),parent,ID))	return false;
	ResetScrollBar();

	return true;
}

DWORD BContainer::GetNrElements()
{
	return vElements.GetSize();
}


bool BContainer::AddHeader(char* headerName,UINT id)
{
	
	BHeader* header;
	header = new BHeader();
	if(id == 0)
	{
		id = ++this->lastId;
	}
	if(!header->Create(headerName,0,this,id)){delete header; return false;}
	if(header == NULL) return false;
	

	
	if(!AddElement(header)){delete header; return false;}
	header->elementType = TYPE_HEADER;
	
	return true;
}
BEdit* BContainer::AddEditBox(char* textBox, unsigned char offset, UINT id)
{
	
	BEdit*	editCtrl;
	
	
	editCtrl = new BEdit();
	if(editCtrl == NULL) return NULL;
	if(id == 0)
	{
		id = ++this->lastId;
	}
	if(!editCtrl->Create(textBox,offset,this,id)){delete editCtrl; return NULL;};

	
	if(!AddElement(editCtrl)){delete editCtrl; return NULL;}
	editCtrl->elementType =  GML::Utils::AttributeList::STRING;

	return editCtrl;

}

BCombo* BContainer::AddComboBox(char* comboTxt,unsigned char offset, UINT id)
{

	BCombo* bCombo;

	bCombo = new BCombo();
	if(bCombo == NULL) return NULL;
	if(id == 0)
	{
		id = ++this->lastId;
	}
	
	if(!bCombo->Create(comboTxt,offset,this,id)){delete bCombo; return NULL;}

		
	if(!AddElement(bCombo)){delete bCombo; return false;}
	bCombo->elementType =  TYPE_COMBO;
	return bCombo;
}

BSelection* BContainer::AddSelection(char* comboTxt,char* btnTxt,unsigned char offset, UINT id)
{
	BSelection* selection;

	selection = new BSelection();
	if(selection == NULL) return NULL;
	if(id == 0)
	{
		id = ++this->lastId;
	}

	if(!selection->Create(comboTxt,btnTxt,offset,this,id)){delete selection; return NULL;}

	if(!AddElement(selection)){delete selection; return false;}
	selection->elementType =  TYPE_COMBO;
	return selection;
}

BFile* BContainer::AddFile(char* comboTxt,unsigned char offset, UINT id, bool isFolder)
{

	BFile* bFile;

	bFile = new BFile();
	if(bFile == NULL) return NULL;
	if(id == 0)
	{
		id = ++this->lastId;
	}
	if(!bFile->Create(comboTxt,offset,this,id,isFolder)){delete bFile; return NULL;}

		
	if(!AddElement(bFile)){delete bFile; return NULL;}

	bFile->elementType =  GML::Utils::AttributeList::STRING;



	return bFile;
}
BButton* BContainer::AddButton(char* str, char* buttonTxt,unsigned char offset, UINT id)
{
	BButton* bButton;

	bButton = new BButton();
	if(bButton == NULL) return NULL;
	if(id == 0)
	{
		id = ++this->lastId;
	}
	
	if(!bButton->Create(str,buttonTxt,offset,this,id)){delete bButton; return NULL;}
	
		
	if(!AddElement(bButton)){delete bButton; return NULL;}
	bButton->elementType =  TYPE_ELEMENT_HEADER;

	return bButton;
}

BCheckBox* BContainer::AddCheckBox(char* comboTxt,unsigned char offset, UINT id)
{

	BCheckBox* bCheck;

	bCheck = new BCheckBox();
	if(bCheck == NULL) return NULL;
	if(id == 0)
	{
		id = ++this->lastId;
	}
	
	if(!bCheck->Create(comboTxt,offset,this,id)){delete bCheck; return NULL;}
	
		
	if(!AddElement(bCheck)){delete bCheck; return NULL;}
	bCheck->elementType =  GML::Utils::AttributeList::BOOLEAN;

	return bCheck;
}


bool BContainer::AddElement(BItem* element)
{
	BItem* parentItem;
	
	if(!vElements.Push(&element)) return false;
	if(vElements.GetSize() > 1)
	{
		//am cel putin doua elemente
		parentItem = (BItem*)	vElements.GetPointer(vElements.GetSize()-2);
		if(parentItem->offset <  element->offset)
		{
			parentItem->SetCollapsed(ITEM_COLLAPSED);
			element->ShowWindow(SW_HIDE);
		}
	}
	ResetScrollBar();

	return true;
}

BContainer::~BContainer(void)
{

}
bool BContainer::SetCollapsedState(int id, int state)
{
	BItem* item = (BItem*)(vElements.GetPointer(id));
	if(item == NULL)
		return false;
	item->SetCollapsed(state);
	return true;
}

bool BContainer::ShowElement(int id,int cmdShow)
{
	
	
	BItem* item = (BItem*)(vElements.GetPointer(id));
	if(item == NULL)
		return false;
	if(!item->ShowWindow(cmdShow))
		return false;
	return true;
	
}
void BContainer::OnPaint()
{
	CRect   cr;

	
	  

	GetClientRect(&cr);
	CPaintDC dc(this);
	CDC* pDC = &dc;
	CDC* pParsedDC = pDC;
	CDC     MemDC;
	CBitmap MemBM;
	//pDC->SetViewportExt(cr.right, cr.bottom);

	//pDC->SetMapMode(MM_ISOTROPIC);
    //pDC->SetViewportOrg(cr.left, cr.bottom);
	//pDC->SetWindowExt(1,1);
	//pDC->SetViewportExt(1,-1);

	MemBM.CreateCompatibleBitmap(pParsedDC,cr.Width(),cr.Height());
	MemDC.CreateCompatibleDC(pParsedDC);
	MemDC.SelectObject(&MemBM);
	pDC = &MemDC;
	
	//umple backgroundul

	CRect fillRect;
    fillRect.SetRect(0,0,cr.Width(),cr.Height());


    pDC->FillSolidRect(&fillRect, RGB(240,240,240));	
	pDC->SetBkMode(TRANSPARENT);
	//fillRect.SetRect(ELEMENT_LEFT+LABEL_WIDTH,0,ELEMENT_LEFT+LABEL_WIDTH+SPACE_BETWEEN_ELEMENT,cr.Height());
	//pDC->FillSolidRect(&fillRect, RGB(220,220,220));
	
	//pDC->DrawEdge(&fillRect,BDR_RAISEDINNER,BF_RECT);

	//pDC->Draw3dRect(o
	
//--------------------------
// flicker
//--------------------------
	//pDC->SetViewportOrg(0,0);
	//MemDC.SetViewportOrg(0,0);

	//verifica daca este vreun element care trebuie collapsed sau nu



	pParsedDC->BitBlt( 0 , 0 , cr.Width() , cr.Height() , &MemDC , 0 , 0 , SRCCOPY );
		
	//CWnd::OnPaint();		
}


BItem* BContainer::GetParent(unsigned int index)
{
	BItem* item,*parent,*element = NULL;

	if(index > vElements.GetSize())
		return NULL;

	item = (BItem*)vElements.GetPointer(index);
	if(item == NULL)
		return false;
	for(int i=0;i<index; i++)
	{
		parent = (BItem*)vElements.GetPointer(i);
		if(parent->offset< item->offset)
			element = parent;
	}

	return element;
}

void BContainer::Reset()
{
	for(int i =0; i<vElements.GetSize(); i++)
	{
		BItem* el = (BItem*)vElements.GetPointer(i);
		if(el!=NULL)
			delete el;
	}
	vElements.DeleteAll();
	lastYpos = 0;
	lastId = 0;
}
void BContainer::ShowChildren()
{
	
	DWORD yPos;
	BItem* item;
	CRect rect;


	yPos = 0;


	for (int i=0;i<vElements.GetSize();i++)
	{

		BItem* parent = GetParent(i);
		item = (BItem*)vElements.GetPointer(i);
		if(parent !=NULL && parent->collapsed == ITEM_COLLAPSED)
		{
		
				
				while(i < vElements.GetSize() && item->offset > parent->offset)
				{
					item->ShowWindow(SW_HIDE);
					i++;
					item = (BItem*)vElements.GetPointer(i);
				}
				i--;
		}
		else
		{
			item->ShowWindow(SW_SHOW);
			yPos=item->MoveElement(yPos)+ELEMENT_SIZE_BETWEEN;
			
			
		}
		
	}
	ResetScrollBar();

	Invalidate();
}

BItem* BContainer::GetElement(int i)
{
	if (i >= vElements.GetSize())
		return NULL;
	return (BItem*)vElements.GetPointer(i);
}
bool BContainer::MoveElement(BItem* el, unsigned int newPos)
{
	BItem* oldParent,*newParent;
	BItem* nextEl;
	bool foundChild = false;
	int i = GetElementPos(el);	
	


	if( i>= newPos)
		i+=1;
	if(!vElements.Insert(&el,newPos))
		return false;

	//verifica daca vechiul parinte mai are copii
	oldParent  = GetParent(i);

	if(vElements.Delete(i)==false)
		return false;
	//verific daca vechiul parinte mai are copii pentru a-i seta starea de collapse
	
	if(oldParent == NULL)
		return true;
	if( i <vElements.GetSize())
	{
		nextEl = (BItem*)vElements.GetPointer(i);
		if(nextEl->offset > oldParent->offset)
			foundChild = true;
		else
			foundChild = false;
	}
	if(!foundChild)
		oldParent->collapsed = ITEM_NO_COLLAPSE;
	else
		oldParent->collapsed = ITEM_COLLAPSED;
	
	
	//verific daca in urma introducerii elementului a aparut un parinte pentru noul element
	newParent = GetParent(newPos);
	if(newParent != NULL)
		newParent->collapsed = ITEM_COLLAPSED;
	return true;

}

int BContainer::GetLastChildPos(BItem* someEl)
{
	BItem* el;
	UINT parentOffset = 0;
	UINT offset;
	bool found = false;
	int i,j;

	for(i = 0;i <vElements.GetSize();i++)
	{
		el = (BItem*) vElements.GetPointer(i);
		if(el == someEl)
		{
			found = true;
			break;
		}
	}
	if(!found)
		return -1;
	
	el = (BItem*) vElements.GetPointer(i);
	offset = el->offset;
	i+=1;
	while(i<vElements.GetSize())
	{
		el = (BItem*) vElements.GetPointer(i);
		if(el->offset <= offset)
			return i-1;
		i++;
	}
	return i-1;
}

int BContainer::GetElementPos(BItem* someEl)
{
	BItem* el;
	UINT parentOffset = 0;;
	for(int i = 0;i <vElements.GetSize();i++)
	{
		el = (BItem*) vElements.GetPointer(i);
		if(el == someEl)
		{
			return i;
		}
	}
	return -1;
}

bool BContainer::DeleteElement(BItem* currentProperty)
{
	BItem* el;
	BItem* parent;
	UINT parentOffset = 0;
	int i;
	i = GetElementPos(currentProperty);
	if(i < 0)
		return false;
	int j = i+1;
	parentOffset = currentProperty->offset;

	while(j<vElements.GetSize())
	{
		el = (BItem*)vElements.GetPointer(j);
		if(el->offset > parentOffset)
		{
			el->ShowWindow(SW_HIDE);
			vElements.Delete(j);
		}
		else
			break;
	}

	el = (BItem*)vElements.GetPointer(i);
	el->ShowWindow(SW_HIDE);
	
	parent = this->GetParent(i);		
	vElements.Delete(i);

	if(!HasChildren(parent))
		parent->collapsed = ITEM_NO_COLLAPSE;
	
	return true;
	

}


bool BContainer::HasChildren(BItem* el)
{
	int i = 0;
	BItem* child;
	for(i=0;i< vElements.GetSize();i++)
	{
		if((BItem*)vElements.GetPointer(i) == el)
		{
			break;
		}
	}
	i+=1;
	child = (BItem*)vElements.GetPointer(i);
	if(child == NULL)
	{
		return false;
	}

	if(child->offset > el->offset)
		return true;
	return false;
}
bool BContainer::DeleteChildren(BItem* currentProperty)
{
	BItem* el;
	UINT parentOffset = 0;
	int i;
	i = GetElementPos(currentProperty);
	if(i < 0)
		return false;
	//am gasit elementul, vad daca are copii
	int j = i+1;
	parentOffset = currentProperty->offset;
	while(j<vElements.GetSize())
	{
		el = (BItem*)vElements.GetPointer(j);
		if(el->offset > parentOffset)
		{
			el->ShowWindow(SW_HIDE);
			vElements.Delete(j);
		}
		else
			break;
	}
	el = GetElement(i);
	el->collapsed = ITEM_NO_COLLAPSE;
		
	return true;
}
void BContainer::ResetScrollBar()
{
	// Retrieve the dimensions of the client area. 
	SCROLLINFO si;
	CRect cr;
	DWORD nrElementeVisibile = 0;

	GetScrollInfo(SB_VERT,&si,SIF_POS | SIF_RANGE | SIF_PAGE | SIF_TRACKPOS);

	GetClientRect(&cr);
	for(int i=0;i<vElements.GetSize();i++)
	{
		BItem* item = (BItem*)vElements.GetPointer(i);
		if(item->IsVisible())
			nrElementeVisibile++;
	}
    // Set the vertical scrolling range and page size
    si.cbSize = sizeof(si); 
	si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS; 
	si.nMin   = 0; 
	si.nMax   = nrElementeVisibile-1;
	si.nPos = 0;
	

    //si.nPage  = 10;
	//si.nPage = si.nMin+si.nPos;
	si.nPage = (cr.Height() / (ELEMENT_HEIGHT + ELEMENT_SIZE_BETWEEN ))+si.nPos;
	SetScrollInfo(SB_VERT, &si, TRUE); 
	//ScrollWindow(0,100,NULL,NULL);

}

void BContainer::OnVScroll(UINT nSBCode,UINT nPos, CScrollBar* pScrollBar)
{
	// Handle vertical scrollbar messages
	// These can be tweaked to better fit the implementation
	int nInc;
	int yPos;
	SCROLLINFO si;


	GetScrollInfo(SB_VERT,&si,SIF_POS | SIF_RANGE | SIF_PAGE);
	yPos = si.nPos;
	nInc = 0;
	switch (nSBCode)
	{
	case SB_TOP: nInc = -si.nPos; break;
	case SB_BOTTOM: nInc = si.nMax-si.nPos; break;
	case SB_LINEUP: 
		if(si.nPos > 0)
			nInc = -1; 
		else 
			nInc = 0;
		break;
	case SB_LINEDOWN:
		if(si.nPage+si.nPos < si.nMax+1)
			nInc = 1; 
		else
			nInc = 0;
		break;
//	case SB_PAGEUP: nInc = min(-1, -m_nVertInc); break;
//	case SB_PAGEDOWN: nInc = max(1, m_nVertInc); break;
	case SB_THUMBTRACK: nInc = nPos - si.nPos; break;
	default: nInc = 0;
	}

//	nInc = max(-si.nPos, min(nInc, si.nMax - si.nPos));

	if (nInc)
	{
		si.nPos += nInc;
		int iMove = (yPos - si.nPos)*(ELEMENT_HEIGHT+ELEMENT_SIZE_BETWEEN);

		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS ;
		SetScrollInfo(SB_VERT,&si,true);

		ScrollWindow(0, iMove, NULL, NULL);
	//	CWnd::UpdateWindow();
		//SetScrollPos(SB_VERT, m_nVscrollPos, TRUE);
		
	}
	

	
	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);

}

BOOL BContainer::OnEraseBkgnd(CDC* pDC) 
{
	// stop background being erased
	return TRUE ;
}


BItem* BContainer::GetSelectedElement()
{
	CWnd* selectedWindow = this->GetFocus();
	CWnd* parent = selectedWindow->GetParent();
	for(int i=0; i< vElements.GetSize(); i++)
	{
		if(parent == ((BItem*)vElements.GetPointer(i)))
		{
			return (BItem*)vElements.GetPointer(i);
		}
	}
	return NULL;
}


bool BContainer::GetElementStr(BItem* element,CString* str)
{
	BCheckBox* boolItem;
	BEdit*	editItem;
	BCombo* comboItem;
	BButton* buttonItem;
	CString temp;
	GString gStrTemp;
	

	UINT someUIntValue;
	int	someIntValue;
	double someDoubleValue;


	switch(element->elementType)
	{
	case GML::Utils::AttributeList::BOOLEAN:
		boolItem = (BCheckBox*)element;
		if(boolItem->IsChecked())
			str->Format("%s=True;",boolItem->label);
		else
			str->Format("%s=False;",boolItem->label);
		return true;

	case GML::Utils::AttributeList::UINT32:
		editItem = (BEdit*)element;
		editItem->GetText(temp);
		gStrTemp.Set(temp.GetBuffer());
		if(!gStrTemp.ConvertToUInt32(&someUIntValue))
			return false;
		str->Format("%s=%u;",editItem->label,someUIntValue);
		return true;
	case GML::Utils::AttributeList::INT32:
		editItem = (BEdit*)element;
		editItem->GetText(temp);
		gStrTemp.Set(temp.GetBuffer());
		if(!gStrTemp.ConvertToInt32(&someIntValue))
			return false;
		str->Format("%s=%d;",editItem->label,someIntValue);
		return true;
	
	case GML::Utils::AttributeList::DOUBLE:
		editItem = (BEdit*)element;
		editItem->GetText(temp);
		gStrTemp.Set(temp.GetBuffer());
		if(!gStrTemp.ConvertToDouble(&someDoubleValue))
			return false;
		str->Format("%s=%lf;",editItem->label,someDoubleValue);
		return true;	
	case GML::Utils::AttributeList::STRING:
		editItem = (BEdit*)element;
		editItem->GetText(temp);
		if(temp.Compare("")==0)
		{
			return false;			
		}
		str->Format("%s=%s;",editItem->label,temp);
		return true;

	case TYPE_COMBO:
		comboItem = (BCombo*)element;
		comboItem->GetSelectedItem(temp);
		str->Format("%s=%s;",comboItem->label,temp);
		return true;
	case TYPE_ELEMENT_HEADER:
		buttonItem = (BButton*)element;
		buttonItem->GetString(&temp);
		str->Format("%s",temp);
		return true;
	}

	return false;
}
bool BContainer::GetConnectorStr(int* i,CString* connector)
{
	BItem* element;
	DWORD parentOffset;
	BButton* conName;
	CString temp;

	
	element = GetElement(*i);
	if(element == NULL)
		return false;
	if(element->elementType != TYPE_ELEMENT_HEADER)
	{
		return false;
	}
	conName = (BButton*)element;
	conName->GetString(&temp);
	parentOffset = element->offset;

	connector->Format("%s{",temp);
	*i+=1;
	element = GetElement(*i);

	while(*i<vElements.GetSize() && element != NULL && element->offset >parentOffset)
	{
		if(!GetElementStr(element,&temp))
		{
			*i+=1;
			element = GetElement(*i);
			continue;
		}
		connector->Append(temp);		
		*i+=1;
		element = GetElement(*i);
	}
	connector->AppendChar('}');
	return true;
}
bool BContainer::GetComplexElement(int* i, CString* str)
{
	BCombo* combo;
	BItem* element;
	CString complexValue;
	CString tempValue;
	CString temp;

	element = (BItem*)vElements.GetPointer(*i);
	if(element == NULL)
		return false;
	combo = (BCombo*)element;
	if(!combo->GetSelectedItem(complexValue))
		complexValue = "";

	*i+=1;
	element = (BItem*)vElements.GetPointer(*i);

	str->SetString(complexValue);
	str->Append("{");
	while(*i<vElements.GetSize() && element != NULL && element->offset >combo->offset)
	{
				
				
		if(i == NULL)
			return false;
		if(!GetElementStr(element,&temp))
		{
			i++;
			element = (BItem*)vElements.GetPointer(*i);
		}				
		str->Append(temp);
		*i+=1;
		element = (BItem*) vElements.GetPointer(*i);
	}
	str->Append("}");
	return true;
}
bool BContainer::GetConnectorsSaveString(int* i,CString* str)
{
	BItem* element;
	DWORD parentOffset = 0;
	CString	tempConnector;

	element = GetElement(*i);
	parentOffset = element->offset;
	if(!HasChildren(element))
	{
		str->SetString("");
		return true;
	}
	str->SetString("");
	*i+=1;
	while(*i<this->GetNrElements())
	{
		element = GetElement(*i);
		if(element->offset <=parentOffset)
			break;
		if(!GetConnectorStr(i,&tempConnector))
			return false;
		str->Append(tempConnector);
		str->Append("=>");
	}
	*i-=1;

	if(str->Right(2).Compare("=>")==0)
		str->SetString(str->Left(str->GetLength() -2));
	return true;
}
bool BContainer::SaveConfigurationToAttributeList(char* algorithmName,GML::Utils::AttributeList*	attrList)
{
	BItem* element,*currentProperty;
	BCombo* combo;
	char* objectName;
	GString gStrTemp;
	CString temp,tempStr;
	CString complexValue;
	CString tempValue;
	CString connectorStr;
	bool	foundComplex;
	char* complexMetaData;
	void* objectValue;
	unsigned char complexDataType;
	unsigned char objectType;
	bool someBoolValue;
	GML::Utils::Attribute*	attr;
	unsigned int someUIntValue;
	int someIntValue;
	double someDoubleValue;
	BEdit* editValue;
	BCombo* comboValue;
	BCheckBox* boolItem;
	BButton* buttonValue;
	BItem* item;


	for(int i=0;i<vElements.GetSize();i++)
	{
		
		element = (BItem*)vElements.GetPointer(i);
		if(element == NULL)
			return false;
		
		objectName = element->label;
		foundComplex = false;

		if(strcmp(objectName,"DataBases") == 0)
		{
			if(!GetComplexElement(&i,&temp))
				return false;
			if(!attrList->AddAttribute("DataBase",(char*)temp.GetString(),GML::Utils::AttributeList::STRING,1,NULL))
			{
				return false;
			}
			continue;
		}
		if(strcmp(objectName,"Notifiers") == 0)
		{
			if(!GetComplexElement(&i,&temp))
				return false;
			if(!attrList->AddAttribute("Notifier",(char*)temp.GetString(),GML::Utils::AttributeList::STRING,1,NULL))
			{
				return false;
			}
			continue;
		}
		if(strcmp(objectName,"Connectors")==0)
		{
			if(!GetConnectorsSaveString(&i,&temp))
				return false;
			if(!attrList->AddAttribute("Connector",(char*)temp.GetString(),GML::Utils::AttributeList::STRING,1,NULL))
			{
				return false;
			}
			continue;
		}
			
		switch(element->elementType)
		{
		case TYPE_HEADER:
			continue;
		case GML::Utils::AttributeList::STRING:
			editValue = (BEdit*)element;
			editValue->GetText(temp);
			objectValue = (char*)temp.GetString();
			objectType = GML::Utils::AttributeList::STRING;
			break;
		case TYPE_COMBO:
			comboValue = (BCombo*)element;
			comboValue->GetSelectedItem(temp);
			objectValue = (char*)temp.GetString();
			objectType = GML::Utils::AttributeList::STRING;
			break;
		case GML::Utils::AttributeList::BOOLEAN:
			boolItem = (BCheckBox*)element;
			someBoolValue = boolItem->IsChecked();
			objectValue = &someBoolValue;
			objectType = element->elementType;
			break;

		case GML::Utils::AttributeList::UINT32:
			editValue = (BEdit*)element;
			editValue->GetText(temp);
			gStrTemp.Set(temp.GetBuffer());
			if(!gStrTemp.ConvertToUInt32(&someUIntValue))
				return false;
			objectValue = &someUIntValue;
			objectType = element->elementType;
		case GML::Utils::AttributeList::INT32:
			editValue = (BEdit*)element;
			editValue->GetText(temp);
			gStrTemp.Set(temp.GetBuffer());
			if(!gStrTemp.ConvertToInt32(&someIntValue))
				return false;
			objectValue = &someIntValue;
			objectType = element->elementType;
			break;
			
	
		case GML::Utils::AttributeList::DOUBLE:
			editValue = (BEdit*)element;
			editValue->GetText(temp);
			gStrTemp.Set(temp.GetBuffer());
			if(!gStrTemp.ConvertToDouble(&someDoubleValue))
				return false;
			objectValue = &someDoubleValue;
			objectType = element->elementType;
			break;	
		default:
			continue;
			

		}
		if(!attrList->AddAttribute(objectName,objectValue,objectType,1,NULL))
		{
			return false;
		}
				
	}

	attr = attrList->Get("AlgorithmName");
	if(attr== NULL)
	{
		if(!attrList->AddAttribute("AlgorithmName",algorithmName,GML::Utils::AttributeList::STRING,1,NULL))
		{
			return false;
		}
	}
	return true;
}

bool BContainer::SetAlgorithmName(char* algName)
{
	if(algName == NULL)
		return false;
	algorithmName = algName;
	return true;
}

bool BContainer::SaveConfiguration(char* algorithmName,char* fileName,GML::Utils::AttributeList*	attrList)
{
	GML::Utils::AttributeList	tempList;
	if (attrList == NULL)
	{
		attrList = &tempList;
		if(!SaveConfigurationToAttributeList(algorithmName,attrList))
		{
			AfxMessageBox("Failed to save atttributeList!");
			return false;
		}
	}

	if(!attrList->Save(fileName))
	{
		AfxMessageBox("Failed to save atttributeList. Please verify file path");
		return false;
	}

	return true;

}