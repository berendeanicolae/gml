#include "stdafx.h"
#include "AlgorithmObject.h"
#define _CRTDBG_MAP_ALLOC



AlgorithmObject::AlgorithmObject()
{
	memset(UIObjects,0,sizeof(UIObjects));
	for(int i=0;i<MAX_NR_OBJECTS;i++)
	{
		UIObjects[i].objectName = NULL;
		UIObjects[i].objectValue = NULL;
	}
	nrObjects = 0;
	algorithmName = NULL;
	propControl = NULL;
	configurationFile = NULL;

	currentDataBase = NULL;
	currentNotifier = NULL;
	currentConnector = NULL;
}

AlgorithmObject::AlgorithmObject(char* algName)
{
	unsigned int algNameSize;
	for(int i=0;i<MAX_NR_OBJECTS;i++)
	{
		UIObjects[i].objectName = NULL;
		UIObjects[i].objectValue = NULL;
	}

	nrObjects = 0;

	algNameSize = strlen(algName);
	algorithmName = new char[algNameSize+1];
	if(algorithmName != NULL)
	{
		strcpy(algorithmName,algName);
	}
	propControl = NULL;

	currentDataBase = NULL;
	currentNotifier = NULL;
	currentConnector = NULL;
	configurationFile = NULL;
	
}

AlgorithmObject::~AlgorithmObject()
{
	for(unsigned int i=0;i< nrObjects;i++)
	{
		if(UIObjects[i].objectValue != NULL)
		{
			delete(UIObjects[i].objectValue);
			UIObjects[i].objectValue = NULL;
		}
		if(UIObjects[i].objectName != NULL)
		{
			delete(UIObjects[i].objectName);
			UIObjects[i].objectName = NULL;
		}

		if(UIObjects[i].metadata != NULL)
		{
			delete(UIObjects[i].metadata);
			UIObjects[i].metadata = NULL;
		}
	}

	if(algorithmName != NULL)
	{
		delete(algorithmName);
		algorithmName = NULL;
	}

	if(configurationFile = NULL)
	{
		delete(configurationFile);
		configurationFile = NULL;
	}


	if(currentNotifier != NULL)
	{
		delete(currentNotifier);
		currentNotifier = NULL;
	}

	if(currentConnector != NULL)
	{
		delete(currentConnector);
		currentConnector = NULL;
	}
}


bool AlgorithmObject::AddMetadata(unsigned int objectNumber,char* description)
{
	unsigned int nameSize;
	char* currentObjectDescription;

	if (objectNumber >= nrObjects)
	{
		return false;
	}
	if(description == NULL)
	{
		UIObjects[objectNumber].metadata = NULL;
		return true;
	}

	nameSize = strlen(description);


	currentObjectDescription = new char[nameSize+1];
	if(currentObjectDescription == NULL)
		return false;
	strcpy(currentObjectDescription, description);
	UIObjects[objectNumber].metadata = currentObjectDescription;
	return true;

}
bool AlgorithmObject::AddName(unsigned int objectNumber,char* name)
{
	unsigned int nameSize;
	char* currentObjectName;

	if (objectNumber >= nrObjects)
	{
		return false;
	}
	if(name == NULL)
	{
		return false;
	}

	nameSize = strlen(name);

	
	currentObjectName = new char[nameSize+1];
	if(currentObjectName == NULL)
		return false;
	strcpy(currentObjectName, name);
	UIObjects[objectNumber].objectName = currentObjectName;
	return true;


}
bool AlgorithmObject::AddFilesFromDirToCombo(char* dirName,char* extension, EPropertyCombo* currentCombo)
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

char* AlgorithmObject::GetObjectName(const void* objectPointer)
{
	for(unsigned int i=0;i<nrObjects;i++)
	{
		if(objectPointer == UIObjects[i].objectValue)
			return UIObjects[i].objectName;
	}
	return NULL;
}

int AlgorithmObject::GetObjectPos(const void* objectPointer)
{
	for(unsigned int i=0;i<nrObjects;i++)
	{
		if(objectPointer == UIObjects[i].objectValue)
			return i;
	}
	return -1;
}
EProperty* AlgorithmObject::AddComboFromDescription(char* name, GML::Utils::GString&	objectDescription, char*	selectedObject)
{
	void* currentObject;
	EPropertyCombo*				currentCombo;
	GML::Utils::GString			textItem;
	int currentPos = 0;
	int selectedItem = 0;
	int numberOfItems = 0;

	currentObject = new int;
	if(currentObject == NULL)
	{
		return NULL;
	}
//	UIObjects[nrObjects++].objectValue = currentObject;
//	if(!AddName(nrObjects-1,attr->Name))
//		return false;

	currentCombo = currentPropList->AddPropCombo(  this , name ,(int*) currentObject );

	
	if(currentCombo== NULL)
	{
		delete(currentObject);
		return NULL;
	}
		
	while(objectDescription.CopyNext(&textItem,",",&currentPos))
	{
		
		if(textItem.Compare(selectedObject,true)==0)
		{
			selectedItem = numberOfItems;
		}
		currentCombo->AddString(textItem.GetText());
		numberOfItems+=1;
	}

	*(int*)currentObject = selectedItem;	
	return currentCombo;
}


EProperty* AlgorithmObject::AddString(GML::Utils::Attribute* attr, char* strValue)
{
	//pur si simplu afisez stringul
	GML::Utils::GString			objectDescription;
	GML::Utils::GString			objectMetadataList;
	void*						currentObject;
	EProperty*					returnedProperty;

	if(attr->AttributeType !=  GML::Utils::AttributeList::STRING )
	{
		return false;
	}
	attr->GetListItem(objectMetadataList);
	attr->GetDescription(objectDescription);

	if(objectMetadataList.GetSize() > 0)
	{
		returnedProperty = AddComboFromDescription(attr->Name,objectMetadataList,strValue);
		if(returnedProperty != NULL)
		{
			UIObjects[nrObjects].objectValue = returnedProperty->m_pProperty;
			UIObjects[nrObjects++].ObjectType = TYPE_COMBO;
		}
	}
	else
	{

		currentObject = new CString;
		if(currentObject == NULL)
		{
			return NULL;
		}
		((CString*)currentObject)->SetString(strValue);
		returnedProperty = currentPropList->AddPropString ( this , attr->Name             ,(CString*)currentObject )->SetComment(objectDescription.GetText());	
		UIObjects[nrObjects].objectValue = currentObject;
		UIObjects[nrObjects++].ObjectType = GML::Utils::AttributeList::STRING;
	}

	
	
	if(!AddName(nrObjects-1,attr->Name))
		return NULL;
	if(!AddMetadata(nrObjects-1,attr->MetaData)){return NULL;}
	return returnedProperty;
};


EProperty* AlgorithmObject::AddBool(GML::Utils::Attribute* attr, unsigned char boolValue)
{
	GML::Utils::GString			str;
	GML::Utils::GString			objectDescription;
	GML::Utils::GString			objectMetadataList;
	void*						currentObject;
	EProperty*					returnedProperty;

	attr->GetDescription(objectDescription);					


	currentObject = new unsigned char;
	if(currentObject == NULL)
	{
		return NULL;
	}
	UIObjects[nrObjects].objectValue = currentObject;
	UIObjects[nrObjects++].ObjectType = GML::Utils::AttributeList::BOOLEAN;
	attr->GetDescription(objectDescription);					

	*(unsigned char*)currentObject = boolValue;
	if(!AddName(nrObjects-1,attr->Name))
		return NULL;
	if(!AddMetadata(nrObjects-1,attr->MetaData)){return NULL;}
	returnedProperty = currentPropList->AddPropCheck ( this , attr->Name             ,(bool*)currentObject )->SetComment(objectDescription.GetText());
	return returnedProperty;
}

EProperty* AlgorithmObject::AddUInt(GML::Utils::Attribute* attr, unsigned int uintValue)
{
	GML::Utils::GString			str;
	GML::Utils::GString			objectDescription;
	GML::Utils::GString			objectMetadataList;
	EProperty*					returnedProperty;	
	void*						currentObject;
	
	currentObject = new unsigned int;
	if(currentObject == NULL)
	{
		return NULL;
	}

	UIObjects[nrObjects].objectValue = currentObject;
	UIObjects[nrObjects++].ObjectType = GML::Utils::AttributeList::UINT32;
	attr->GetDescription(objectDescription);					

	*(unsigned int*)currentObject = uintValue;
	if(!AddName(nrObjects-1,attr->Name))
		return NULL;
	if(!AddMetadata(nrObjects-1,attr->MetaData)){return NULL;}
	returnedProperty = currentPropList->AddPropInt ( this , attr->Name             ,(unsigned int*)currentObject )->SetComment(objectDescription.GetText());			
	return returnedProperty;
}


EProperty* AlgorithmObject::AddInt(GML::Utils::Attribute* attr, int intValue)
{
	GML::Utils::GString			str;
	GML::Utils::GString			objectDescription;
	GML::Utils::GString			objectMetadataList;
	void*						currentObject;
	EProperty*					returnedProperty;

		currentObject = new int;
	if(currentObject == NULL)
	{
		return NULL;
	}

	UIObjects[nrObjects].objectValue = currentObject;
	UIObjects[nrObjects++].ObjectType = GML::Utils::AttributeList::INT32;
	attr->GetDescription(objectDescription);					

	*(int*)currentObject = intValue;
	if(!AddName(nrObjects-1,attr->Name))
		return NULL;
	if(!AddMetadata(nrObjects-1,attr->MetaData)){return NULL;}
	returnedProperty = currentPropList->AddPropInt ( this , attr->Name             ,(int*)currentObject )->SetComment(objectDescription.GetText());			
	return returnedProperty;
}


EProperty* AlgorithmObject::AddDouble(GML::Utils::Attribute* attr, double dblValue)
{
	GML::Utils::GString			str;
	GML::Utils::GString			objectDescription;
	GML::Utils::GString			objectMetadataList;
	void*						currentObject;
	EProperty*					returnedProperty;

	currentObject = new double;
	if(currentObject == NULL)
	{
		return NULL;
	}
	UIObjects[nrObjects].objectValue = currentObject;
	UIObjects[nrObjects++].ObjectType = GML::Utils::AttributeList::DOUBLE;
	attr->GetDescription(objectDescription);					

	*(double*)currentObject = dblValue;
	if(!AddName(nrObjects-1,attr->Name))
		return NULL;
	if(!AddMetadata(nrObjects-1,attr->MetaData)){return NULL;}
	returnedProperty = currentPropList->AddPropDouble ( this , attr->Name             ,(double*)currentObject )->SetComment(objectDescription.GetText());	
	return returnedProperty;
}


bool AlgorithmObject::AddSpecialCombo(char* comboName,char* extension,GML::Utils::Attribute* attr,GML::Utils::GString& str)
{
	EPropertyCombo*				currentCombo;
	void*						currentObject;
	GML::Utils::GString			objectDescription;
	GML::Utils::GString			selectedObject;
	GML::Utils::GString			propertyName;
	GML::Utils::GString			propData[2];
	int							selectedIndex;
	GML::Utils::AttributeList	objectProperties;
	

	if(comboName == NULL)
		return false;
	if(extension == NULL)
		return false;


	currentObject = new int;
	if(currentObject == NULL)
	{
		return false;
	}
	UIObjects[nrObjects].objectValue = currentObject;
	UIObjects[nrObjects++].ObjectType = TYPE_COMBO;


	if(!AddName(nrObjects-1,comboName))
		return false;
	if(!AddMetadata(nrObjects-1,attr->MetaData)){return NULL;}
	currentCombo = currentPropList->AddPropCombo(  this , comboName ,(int*) currentObject );
	if(currentCombo!= NULL)
	{
		if(!AddFilesFromDirToCombo(comboName,extension,currentCombo))
		{
			currentCombo->AddString("Didn't find any connector");
		}

	}	
	attr->GetDescription(objectDescription);	
	currentCombo->SetComment(objectDescription.GetText());	
	
	//incarca intai obiectul setat
	selectedIndex = -1;
	int pos = 0;
	if(str.GetSize() > 1)
	{
		str.Strip(" \t\n\r\"");
		str.CopyNext(&selectedObject,"{",&pos);
		if(selectedObject.GetSize() > 0)
		{
			for(int i=0;i<currentCombo->GetItemCount();i++)
			{
				if(selectedObject.Compare((char*)currentCombo->GetItemText(i).GetString(),true) == 0)
				{
					selectedIndex = i;
					break;
				}
					
			}
			
			if(selectedIndex != -1)
			{
				*((int*)currentCombo->m_pProperty) = selectedIndex;
				//construieste lista de atribute
				
				if(str.GetText()[str.GetSize()-1] == '}');
					str.ReplaceOnPos(str.GetSize()-2,str.GetSize()-1,";");

				//
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
				propControl->UpdatePropertyHost();

			}
		}
	}






	return true;
}

bool AlgorithmObject::LoadAlgorithm(EPropList& PropList)
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
	EPropertyCombo*				currentCombo;

	bool						boolValue;
	UInt32						uint32Value;
	Int32						int32Value;
	double						doubleValue;
	void*						currentObject;


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
	

	//incarc intai alea standard...
/*	
	attr = attrList.Get("Notifier");
	if(attr!= NULL)
	{
		attrList.UpdateString(attr->Name,str);
		if(!AlgorithmObject::AddSpecialCombo("Notifiers","ntf",attr,str)){return false;}
		
		attr = attrList.Get("DataBase");
		if(attr != NULL)
		{
			attrList.UpdateString(attr->Name,str);
			if(!AlgorithmObject::AddSpecialCombo("DataBases","db",attr,str)){return false;}
			
			attr = attrList.Get("Connector");
			if(attr!=NULL)
			{
				attrList.UpdateString(attr->Name,str);
				//sterge
				if(!AddString(attr,str)){return false;}
				//temporar
				//if(!AlgorithmObject::AddSpecialCombo("Connectors","dbc",attr,str)){return false;}
			}

		}

	}

*/	
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
				if(!AlgorithmObject::AddSpecialCombo("Connectors","dbc",attr,str)){return false;}
				break;
			}
			
			strResult = _tcsicmp(attr->Name,_T("DataBase"));
			if(strResult == 0)
			{
			
				if(!AlgorithmObject::AddSpecialCombo("DataBases","db",attr,str)){return false;}
				break;
			}

			strResult = _tcsicmp(attr->Name,_T("Notifier"));
			if(strResult == 0)
			{
				if(!AlgorithmObject::AddSpecialCombo("Notifiers","ntf",attr,str)){return false;}
				break;
			}
									
			//pur si simplu afisez stringul
			if(!AddString(attr, str)){return false;}
			break;

		};
	}

	return true;
	
}

void AlgorithmObject::GetProperties( EPropList& PropList )
{
	currentPropList = &PropList;
	PropList.AddTab("Algorithm");
	LoadAlgorithm(PropList);

}

void AlgorithmObject::RemoveObjectFromUIObjects(int objectPos)
{
	if(objectPos < 0)
		return;
	if(objectPos >= nrObjects)
		return;
	if(UIObjects[objectPos].objectValue != NULL)
	{
		delete (UIObjects[objectPos].objectValue);
		UIObjects[objectPos].objectValue = NULL;
	}

	if(UIObjects[objectPos].objectName != NULL)
	{
		delete (UIObjects[objectPos].objectName);
		UIObjects[objectPos].objectName = NULL;
	}

	for(int i=objectPos;i<nrObjects-1;i++)
	{
		UIObjects[i]=UIObjects[i+1];
	}

	nrObjects-=1;
}
bool AlgorithmObject::SaveConfigurationToAttributeList(GML::Utils::AttributeList*	attrList)
{
	EProperty* currentProerty;
	EPropList::ETabGroup* currentTab;
	GML::Utils::Attribute		*attr;
	char* objectName;
	char* objectMetaData;
	CString temp;
	CString complexValue;
	CString tempValue;
	int		currentIndent;
	bool	foundComplex;
	char* complexMetaData;
	void* objectValue;
	unsigned char complexDataType;
	unsigned char objectType;


	if(currentPropList->GetTabCount() <=0)
		return false;

	currentTab = currentPropList->GetTab(0);
	if(currentTab == NULL)
		return false;

	for(int i=0;i<nrObjects;i++)
	{
		objectName = UIObjects[i].objectName;
		foundComplex = false;
		if(strcmp(objectName,"DataBases") == 0)
		{
			foundComplex = true;
			objectName = "DataBase";
		}

		if(strcmp(objectName,"Notifiers") == 0)
		{
			foundComplex = true;
			objectName = "Notifier";
		}
		if(strcmp(objectName,"Connectors")==0)
		{
			//		decomenteaza
			//		foundComplex = true;
			objectName = "Connector";
		}

		if(foundComplex)
		{

			//avem un obiect mai complex, care are copii. Ex DataBase
			//obiectul complex e sigur de tip Combo


			currentProerty =  currentTab->Get(i);
			complexValue = (((EPropertyCombo*)currentProerty)->GetTextValue()).GetString();
			complexMetaData = UIObjects[i].metadata;
			complexDataType = GML::Utils::AttributeList::STRING;

			i+=1;
			currentProerty =  currentTab->Get(i);

			tempValue = "{";
			while(i<nrObjects && currentProerty != NULL && currentProerty->m_nIndent >0)
			{
				temp="";
				switch(UIObjects[i].ObjectType)
				{
				case GML::Utils::AttributeList::BOOLEAN:
					if(*(unsigned char*)UIObjects[i].objectValue == 1)
						temp.Format("%s=True;",UIObjects[i].objectName);
					if(*(unsigned char*)UIObjects[i].objectValue == 0)
						temp.Format("%s=False;",UIObjects[i].objectName);
					break;

				case GML::Utils::AttributeList::UINT32:
					temp.Format("%s=%u;",UIObjects[i].objectName,*((unsigned int*)UIObjects[i].objectValue));
					break;
				case GML::Utils::AttributeList::INT32:
					temp.Format("%s=%d;",UIObjects[i].objectName,*((int*)UIObjects[i].objectValue));
					break;	
				case GML::Utils::AttributeList::DOUBLE:
					temp.Format("%s=%lf;",UIObjects[i].objectName,*((double*)UIObjects[i].objectValue));
					break;	
				case GML::Utils::AttributeList::STRING:
					if(((CString*)UIObjects[i].objectValue)->Compare("")!=0)
					{
						temp.Format("%s=%s;",UIObjects[i].objectName,((CString*)UIObjects[i].objectValue)->GetString());
					}
					break;

				case TYPE_COMBO:
					temp.Format("%s=%s;",UIObjects[i].objectName,(((EPropertyCombo*)currentProerty)->GetTextValue()).GetString());
					break;

				}
				tempValue+=temp;
				i++;
				currentProerty =  currentTab->Get(i);
			}
			tempValue +="}";
			if(tempValue.GetLength() > 2)
			{
				complexValue+=tempValue;
			}
			i-=1;

		}
		if(foundComplex)
		{
			if(!attrList->AddAttribute(objectName,(char*)complexValue.GetString(),complexDataType,1,NULL))
			{
				return false;
			}
		}
		else
		{
			switch(UIObjects[i].ObjectType)
			{
			case GML::Utils::AttributeList::STRING:
				objectValue = (char*)((CString*)UIObjects[i].objectValue)->GetString();
				objectType = GML::Utils::AttributeList::STRING;
				break;
			case TYPE_COMBO:
				currentProerty =  currentTab->Get(i);
				objectValue = (char*)(((EPropertyCombo*)currentProerty)->GetTextValue()).GetString();
				objectType = GML::Utils::AttributeList::STRING;
				break;
			default:
				objectValue = UIObjects[i].objectValue;
				objectType = UIObjects[i].ObjectType;
				break;

			}
			if(!attrList->AddAttribute(objectName,objectValue,objectType,1,UIObjects[i].metadata))
			{
				return false;
			}
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

bool AlgorithmObject::SaveCurrentConfiguration(char* filePath,GML::Utils::AttributeList*	attrList)
{
	
	GML::Utils::AttributeList	tempList;
	if (attrList == NULL)
	{
		attrList = &tempList;
		if(!SaveConfigurationToAttributeList(attrList))
		{
			AfxMessageBox("Failed to save atttributeList!");
			return false;
		}
	}

	if(!attrList->Save(filePath))
	{
		AfxMessageBox("Failed to save atttributeList. Please verify file path");
		return false;
	}

	return true;

}



bool AlgorithmObject::DeleteChildren(EProperty* parentProperty)
{
	int objectPos;
	int parentIndent;
	EProperty* currentProperty;
	
	
	
	if(!parentProperty->m_bHasChildren)
		return true;
	objectPos = currentPropList->GetPropertyPos(parentProperty);
	parentIndent = parentProperty->m_nIndent;
	
	//incep de la urmatorul element;
	objectPos+=1;
	
	while(objectPos < currentPropList->GetTab(currentPropList->GetActiveTab())->Size() )
	{
		currentProperty = currentPropList->GetTab(currentPropList->GetActiveTab())->Get(objectPos);
		if(currentProperty->m_nIndent <= parentIndent)
			break;
		RemoveObjectFromUIObjects(objectPos);
		currentPropList->GetTab(currentPropList->GetActiveTab())->RemovePropery(objectPos);
		//objectPos+=1;
	}
	
	return true;

}
bool AlgorithmObject::CreateNotifier(EProperty* currentProperty,int* pNewValue)
{
	EPropertyCombo* notifierCombo;
	CString selectedNotifier;

	notifierCombo = (EPropertyCombo*)currentProperty;
	selectedNotifier = notifierCombo->GetItemText(*pNewValue);

	if(currentNotifier != NULL)
	{
		delete(currentNotifier);
		currentNotifier = NULL;
	}

	currentNotifier = GML::Builder::CreateNotifier((char*)selectedNotifier.GetString());
	if(currentNotifier == NULL)
	{
		return false;
	}
	return true;


}

bool AlgorithmObject::MoveUIObject(int oldPos, int newPos)
{
	UIObject temp;
	
	if(oldPos == newPos)
		return true;
	if(oldPos <0 || oldPos>=nrObjects)
		return false;
	if(newPos <0 || newPos>=nrObjects)
		return false;

	temp = UIObjects[oldPos];
	if(newPos < oldPos)
	{
		for(int i=oldPos; i> newPos;i--)
			UIObjects[i]= UIObjects[i-1];
		UIObjects[newPos] = temp;

	}
	else
	{
		for(int i=oldPos;i<newPos;i++)
			UIObjects[i] = UIObjects[i+1];
		UIObjects[newPos] = temp;
	}

	return true;
}
bool AlgorithmObject::MoveProperty(EProperty* prop,int newPos, int indent)
{
		int oldPos;
		if(indent<-1)
			return false;
		oldPos = GetObjectPos(prop->m_pProperty);
		
		if(!MoveUIObject(oldPos, newPos))
			return false;
		if(!currentPropList->MoveProperty(prop,newPos,indent))
			return false;
		return true;
}

bool AlgorithmObject::CreateChildren(EProperty* currentProperty,GML::Utils::AttributeList&	attrList)
{
	CString						temp;
	unsigned int				strResult;
	GML::Utils::Attribute		*attr;
	GML::Utils::GString			str;
	GML::Utils::GString			objectDescription;
	GML::Utils::GString			objectMetadataList;
	int							currentPos;

	bool						boolValue;
	UInt32						uint32Value;
	Int32						int32Value;
	double						doubleValue;
	void*						currentObject;
	EProperty*					createdProperty;

	
	if(!DeleteChildren(currentProperty))
		return false;

	currentPos = currentPropList->GetTab(currentPropList->GetActiveTab())->GetPropertyPos(currentProperty);
	//adaug de la urmatoarea pozitie.
	currentPos+=1;

	for (int tr=0;tr<attrList.GetCount();tr++)
	{
		attr = attrList.Get(tr);

		switch (attr->AttributeType)
		{	

		case GML::Utils::AttributeList::BOOLEAN:
			attrList.UpdateBool(attr->Name,boolValue);
			createdProperty = AddBool(attr, boolValue);
			if(createdProperty == NULL){return false;}
			MoveProperty(createdProperty,currentPos,1);
			currentPos++;

			break;

		case GML::Utils::AttributeList::UINT32:

			attrList.UpdateUInt32(attr->Name,uint32Value);
			createdProperty = AddUInt(attr,uint32Value);
			if(createdProperty == NULL){return false;}
			MoveProperty(createdProperty,currentPos,1);
			currentPos++;
			break;	
		case GML::Utils::AttributeList::INT32:

			attrList.UpdateInt32(attr->Name,int32Value);
			createdProperty = AddInt(attr,int32Value);
			if(createdProperty == NULL){return false;}
			MoveProperty(createdProperty,currentPos,1);
			currentPos++;
			break;	
		case GML::Utils::AttributeList::DOUBLE:
			attrList.UpdateDouble(attr->Name,doubleValue);
			createdProperty = AddDouble(attr,doubleValue);
			if(createdProperty == NULL){return false;}
			MoveProperty(createdProperty,currentPos,1);
			currentPos++;
			break;	
		case GML::Utils::AttributeList::STRING:
			attrList.UpdateString(attr->Name,str);
			//pur si simplu afisez stringul
			createdProperty = AddString(attr, str);
			if(createdProperty == NULL){return false;}
			MoveProperty(createdProperty,currentPos,1);
			currentPos++;
			break;

		};
	}
	return true;
}

bool AlgorithmObject::AddChildrenFromObject(EProperty* currentProperty,int* pNewValue,GML::Utils::AttributeList* newAttrList)
{

	EPropertyCombo* objectCombo;
	GML::Utils::AttributeList	attrList;
	GML::Utils::AttributeList	finalList;
	GML::Utils::Attribute* currentAttribute,*testAttribute;
	CString selectedString;

	objectCombo = (EPropertyCombo*)currentProperty;
	selectedString = objectCombo->GetItemText(*pNewValue);


	GML::Builder::GetPluginProperties((char*)((selectedString).GetString()),attrList);

	if(newAttrList != NULL)
	{
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


bool AlgorithmObject::PropertyChanging( const void* pProperty , void* pNewValue )
{
	
	GML::DB::IDataBase			*dataBase = NULL;
	char* name;
	int pos;
	int stringResult;
	EProperty* myProperty;
	EProperty* currentProperty;
	int dataBasePos;


	name = GetObjectName(pProperty);
	currentProperty = currentPropList->GetProperty(name);
	//pos = GetObjectPos(pProperty);
	stringResult = strcmp(name,"DataBases");
	//verific DataBase

 	if(!strcmp(name,"DataBases") || !strcmp(name,"Notifiers") || !strcmp(name,"Connectors"))
	{
		if(!AddChildrenFromObject(currentProperty,(int*)pNewValue,NULL))
			return false;
		propControl->UpdatePropertyHost();
		return true;
	}
	return true;
}

bool AlgorithmObject::SetPropertyControl(EPropCtrl *newPropControl)
{
	if(newPropControl != NULL)
	{
		propControl = newPropControl;
		return true;
	}
	return false;

}

bool AlgorithmObject::IsPropertyEnabled( const void* pProperty )
{
	bool bEnabled = true;
	char* name;

	name = this->GetObjectName(pProperty);
	
/*
	if( pProperty==&m_dWidth || pProperty==&m_dHeight )
	{
		bEnabled = m_bSizeEnabled;
	}

	return bEnabled;
*/
	return true;
}



bool AlgorithmObject::SetConfigurationFile(char* configFile)
{
	unsigned int length;

	if(configurationFile != NULL)
	{
		delete(configurationFile);
		configurationFile = NULL;
	}

	length = strlen(configFile);
	configurationFile = new char[length+1];
	if(configurationFile == NULL)
		return false;

	strcpy(configurationFile,configFile);
	return true;


}
