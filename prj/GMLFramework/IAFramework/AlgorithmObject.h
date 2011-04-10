

#ifndef ALGORITHM_OBJECT
#define ALGORITHM_OBJECT
#pragma once
#include "IPropertyHost.h"
#include "gml\gmllib.h"
#include "EPropCtrl.h"



#define MAX_NR_OBJECTS 100

#define TYPE_COMBO 100




typedef struct 
{
	void* objectValue;
	char* objectName;
	char* metadata;
	unsigned char ObjectType;
}UIObject;

class AlgorithmObject : public IPropertyHost
{

public:
	
	char* algorithmName;
	char* configurationFile;
private:
	UIObject UIObjects[MAX_NR_OBJECTS];
	EPropList* currentPropList;
	GML::DB::IDataBase* currentDataBase;
	GML::Utils::INotifier* currentNotifier;
	GML::ML::IConnector* currentConnector;
	

	unsigned int nrObjects;
	

	EPropCtrl* propControl;

//temp
	CString m_sTest;

public:

	AlgorithmObject();
	AlgorithmObject(char* algName);
	virtual ~AlgorithmObject();




	virtual void GetProperties( EPropList& PropList );
	virtual bool PropertyChanging( const void* pProperty , void* pNewValue );
	virtual bool IsPropertyEnabled( const void* pProperty );

	bool LoadAlgorithm(EPropList& PropList);
	bool AddFilesFromDirToCombo(char* dirName,char* extension, EPropertyCombo* currentCombo);
	bool AddName(unsigned int objectNumber,char* name);
	char* GetObjectName(const void* objectPointer);
	int GetObjectPos(const void* objectPointer);
	bool SetPropertyControl(EPropCtrl *newPropControl);
		//currentAlgorithm->SetPropertyControl(&m_PropCtrl);
	bool DeleteChildren(EProperty* parentProperty);
	void RemoveObjectFromUIObjects(int objectPos);
	
	EProperty* AddComboFromDescription(char* name, GML::Utils::GString&	objectDescription, char*	selectedObject);
	bool AddChildrenFromDataBase(EProperty* currentProperty,int* pNewValue);
	bool AddChildrenFromNotifier(EProperty* currentProperty,int* pNewValue);
	bool AddChildrenFromConnector(EProperty* currentProperty,int* pNewValue);
	bool AddChildrenFromObject(EProperty* currentProperty,int* pNewValue,GML::Utils::AttributeList* newAttrList);
	
	bool CreateNotifier(EProperty* currentProperty,int* pNewValue);

	EProperty* AlgorithmObject::AddString(GML::Utils::Attribute* attr, char* strValue);
	EProperty* AlgorithmObject::AddBool(GML::Utils::Attribute* attr, unsigned char boolValue);
	EProperty* AlgorithmObject::AddInt(GML::Utils::Attribute* attr,  int intValue);
	EProperty* AlgorithmObject::AddUInt(GML::Utils::Attribute* attr, unsigned int intValue);
	EProperty* AlgorithmObject::AddDouble(GML::Utils::Attribute* attr, double dblValue);
	bool MoveProperty(EProperty* prop,int pos, int indent = -1);
	bool MoveUIObject(int oldPos, int newPos);
	bool CreateChildren(EProperty* currentProperty,GML::Utils::AttributeList&	attrList);
	//bool SaveCurrentConfiguration(char* filePath);
	bool SaveCurrentConfiguration(char* filePath,GML::Utils::AttributeList*	attrList= NULL);
	bool SaveConfigurationToAttributeList(GML::Utils::AttributeList*	attrList);
	bool AddMetadata(unsigned int objectNumber,char* description);

	bool SetConfigurationFile(char* configFile);
	bool AddSpecialCombo(char* comboName,char* extension,GML::Utils::Attribute* attr, GML::Utils::GString& str);
		 

	



};



#endif