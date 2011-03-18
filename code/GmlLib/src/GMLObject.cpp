#include "GMLObject.h"


#define TEST_SET_PROP(attr_sursa_name,tip,tip_sursa) \
				if (attr->AttributeType==attr_sursa_name) \
				{ \
					(*((tip *)link->LocalAddress)) = (tip)(*((tip_sursa *)attr->Data)); \
					break; \
				} 
#define TEST_SETLIST_PROP(tip) \
				if (AttributeToListIndex(attr,link,&int64Value)) \
				{ \
					(*((tip *)link->LocalAddress)) = (tip)int64Value; \
					break; \
				}

bool AttributeToListIndex(GML::Utils::Attribute *attr,GML::Utils::AttributeLink	*link,Int64 *intValue)
{
	GML::Utils::GString		list,item,value;
	int						poz,p_eq;
	TYPE_INT64				cValue;

	if (GML::Utils::GString::StartsWith(link->MetaData,"!!LIST:",true)==false)
		return false;
	if ((poz=GML::Utils::GString::Find(&link->MetaData[7],"!!"))==-1)
		return false;
	if (list.Set(&link->MetaData[7],poz)==false)
		return false;
	poz = 0;
	cValue = 0;
	while (list.CopyNext(&item,",",&poz))
	{
		if ((p_eq=item.FindLast("="))!=-1)
		{
			if (value.Set(&item.GetText()[p_eq+1])==false)
				return false;
			item.Truncate(p_eq);
			if (value.ConvertToInt64(&cValue)==false)
				continue;
		} 
		if (item.Equals((char *)attr->Data,true))
		{
			(*intValue) = cValue;
			return true;
		}
		cValue++;
	}
	return false;
}
bool ListIndexToAttribute(GML::Utils::AttributeLink	*link,Int64 intValue,GML::Utils::GString *name)
{
	GML::Utils::GString		list,item,value;
	int						poz,p_eq;
	TYPE_INT64				cValue;

	if (GML::Utils::GString::StartsWith(link->MetaData,"!!LIST:",true)==false)
		return false;
	if ((poz=GML::Utils::GString::Find(&link->MetaData[7],"!!"))==-1)
		return false;
	if (list.Set(&link->MetaData[7],poz)==false)
		return false;
	poz = 0;
	cValue = 0;
	while (list.CopyNext(&item,",",&poz))
	{
		if ((p_eq=item.FindLast("="))!=-1)
		{
			if (value.Set(&item.GetText()[p_eq+1])==false)
				return false;
			item.Truncate(p_eq);
			if (value.ConvertToInt64(&cValue)==false)
				continue;
		} 
		if (cValue==intValue)
		{
			return name->Set(&item);
		}
		cValue++;
	}
	return false;
}
//=========================================================================================================================================================
GML::Utils::GMLObject::GMLObject()
{
	ObjectName = "GMLObject";
	AttrLinks.DeleteAll();
}
bool GML::Utils::GMLObject::LinkPropertyToString(char *Name,GML::Utils::GString &LocalAddr,char *defaultValue,char *Description)
{
	GML::Utils::AttributeLink	link;

	if (LocalAddr.Set(defaultValue)==false)
		return false;
	link.MetaData = Description;
	link.Name = Name;
	link.AttributeType = GML::Utils::AttributeList::STRING;
	link.LocalAddress = &LocalAddr;

	return AttrLinks.PushByRef(link);
}
bool GML::Utils::GMLObject::LinkPropertyToBool(char *Name,bool &LocalAddr,bool defaultValue,char *Description)
{
	GML::Utils::AttributeLink	link;

	LocalAddr = defaultValue;
	link.MetaData = Description;
	link.Name = Name;
	link.AttributeType = GML::Utils::AttributeList::BOOLEAN;
	link.LocalAddress = &LocalAddr;

	return AttrLinks.PushByRef(link);
}
bool GML::Utils::GMLObject::LinkPropertyToDouble(char *Name,double &LocalAddr,double defaultValue,char *Description)
{
	GML::Utils::AttributeLink	link;

	LocalAddr = defaultValue;
	link.MetaData = Description;
	link.Name = Name;
	link.AttributeType = GML::Utils::AttributeList::DOUBLE;
	link.LocalAddress = &LocalAddr;

	return AttrLinks.PushByRef(link);
}
bool GML::Utils::GMLObject::LinkPropertyToUInt32(char *Name,UInt32 &LocalAddr,UInt32 defaultValue,char *Description)
{
	GML::Utils::AttributeLink	link;

	LocalAddr = defaultValue;
	link.MetaData = Description;
	link.Name = Name;
	link.AttributeType = GML::Utils::AttributeList::UINT32;
	link.LocalAddress = &LocalAddr;

	return AttrLinks.PushByRef(link);
}
bool GML::Utils::GMLObject::LinkPropertyToInt32(char *Name,Int32 &LocalAddr,Int32 defaultValue,char *Description)
{
	GML::Utils::AttributeLink	link;

	LocalAddr = defaultValue;
	link.MetaData = Description;
	link.Name = Name;
	link.AttributeType = GML::Utils::AttributeList::INT32;
	link.LocalAddress = &LocalAddr;

	return AttrLinks.PushByRef(link);
}
bool GML::Utils::GMLObject::SetProperyMetaData(char *Name,char *MetaData)
{
	GML::Utils::AttributeLink	*link;
	UInt32						tr;

	for (tr=0;tr<AttrLinks.Len();tr++)
		if ((link=(GML::Utils::AttributeLink *)AttrLinks.GetPtrToObject(tr))!=NULL)
		{
			if (GML::Utils::GString::Equals(link->Name,Name,true))
			{
				link->MetaData = MetaData;
				return true;
			}
		}
	return true;
}
bool GML::Utils::GMLObject::RemoveProperty(char *Name)
{
	GML::Utils::AttributeLink	*link;
	UInt32						tr;

	for (tr=0;tr<AttrLinks.Len();tr++)
		if ((link=(GML::Utils::AttributeLink *)AttrLinks.GetPtrToObject(tr))!=NULL)
		{
			if (GML::Utils::GString::Equals(link->Name,Name,true))
				return AttrLinks.Delete(tr);
		}
	return true;
}
bool GML::Utils::GMLObject::SetProperty(GML::Utils::AttributeList &config)
{
	UInt32						tr;
	GML::Utils::AttributeLink	*link;
	GML::Utils::Attribute		*attr;
	Int64						int64Value;	

	// caut in linkuri daca am ceva similar
	for (tr=0;tr<AttrLinks.Len();tr++)
	{
		if ((link=AttrLinks.GetPtrToObject(tr))==NULL)
		{
			DEBUGMSG("IAlgorithm: Internal Error reading AttrLinkPropertyTo %d ",tr);
			return false;
		}
		// daca nu am acel atribut , continu
		if ((attr = config.Get(link->Name))==NULL)
			continue;
		if (attr->ElementsCount!=1)
		{
			DEBUGMSG("No vectors allowes at this point (on %s)",link->Name);
			return false;
		}

		// verific daca tipul e ok
		switch (link->AttributeType)
		{			
			case GML::Utils::AttributeList::STRING:
				if (attr->AttributeType!=GML::Utils::AttributeList::STRING)
				{
					DEBUGMSG("Invalid Attribute type (expecting string) for %s ",link->Name);
					return false;
				}
				if (((GML::Utils::GString *)link->LocalAddress)->Set((char *)attr->Data,attr->DataSize)==false)
				{
					DEBUGMSG("Error copying string for %s ",link->Name);
					return false;
				}
				break;
			case GML::Utils::AttributeList::BOOLEAN:
				TEST_SET_PROP(GML::Utils::AttributeList::BOOLEAN,bool,bool);
				DEBUGMSG("Expecting a boolean value for %s ",link->Name);
				return false;
			case GML::Utils::AttributeList::DOUBLE:
				TEST_SET_PROP(GML::Utils::AttributeList::DOUBLE,double,double);
				TEST_SET_PROP(GML::Utils::AttributeList::UINT32,double,UInt32);
				TEST_SET_PROP(GML::Utils::AttributeList::INT32,double,Int32);
				TEST_SETLIST_PROP(double);
				DEBUGMSG("Expecting a numerical value (double,uint32,int32 or list) for %s ",link->Name);
				return false;
			case GML::Utils::AttributeList::UINT32:
				TEST_SET_PROP(GML::Utils::AttributeList::UINT32,UInt32,UInt32);
				TEST_SET_PROP(GML::Utils::AttributeList::INT32,UInt32,Int32);
				TEST_SETLIST_PROP(UInt32);
				DEBUGMSG("Expecting a numerical value (uint32,int32 or list) for %s ",link->Name);
				return false;
			case GML::Utils::AttributeList::INT32:
				TEST_SET_PROP(GML::Utils::AttributeList::INT32,Int32,Int32);
				TEST_SET_PROP(GML::Utils::AttributeList::UINT32,Int32,UInt32);
				TEST_SETLIST_PROP(Int32);
				DEBUGMSG("Expecting a numerical value (uint32,int32 or list) for %s ",link->Name);
				return false;
			default:				
				DEBUGMSG("Don`t know to convert %s (unknown type) ",link->Name);
				return false;
		}
	}
	// all ok :-> am setat datele
	return true;
}
bool GML::Utils::GMLObject::SetProperty(char *str_config)
{
	GML::Utils::AttributeList	config;

	if (config.Create(str_config)==false)
	{		
		DEBUGMSG("Unable to create AttributeList from: %s",str_config);
		return false;
	}
	return SetProperty(config);
}
bool GML::Utils::GMLObject::SetPropertyFromFile(char *fileName)
{
	GML::Utils::AttributeList	config;

	if (config.Load(fileName)==false)
	{		
		DEBUGMSG("Unable to load AttributeList from: %s",fileName);
		return false;
	}
	return SetProperty(config);
}
bool GML::Utils::GMLObject::GetProperty(GML::Utils::AttributeList &config)
{
	UInt32						tr;
	GML::Utils::AttributeLink	*link;
	GML::Utils::GString			listValue;
	
	config.Clear();
	for (tr=0;tr<AttrLinks.Len();tr++)
	{
		if ((link=AttrLinks.GetPtrToObject(tr))==NULL)
		{			
			DEBUGMSG("IAlgorithm: Internal Error reading AttrLinkPropertyTo %d ",tr);
			return false;
		}
		switch (link->AttributeType)
		{
			case GML::Utils::AttributeList::STRING:
				if (config.AddString(link->Name,(*(GML::Utils::GString *)link->LocalAddress).GetText(),link->MetaData)==false)
				{
					DEBUGMSG("Error adding : %s ",link->Name);
					return false;
				}
				break;
			case GML::Utils::AttributeList::DOUBLE:
				if (ListIndexToAttribute(link,(Int64)(*(double *)link->LocalAddress),&listValue))
				{
					if (config.AddString(link->Name,listValue.GetText(),link->MetaData)==false)
					{
						DEBUGMSG("Error adding : %s ",link->Name);
						return false;
					}
					break;
				}
				if (config.AddDouble(link->Name,(*(double *)link->LocalAddress),link->MetaData)==false)
				{
					DEBUGMSG("Error adding : %s ",link->Name);
					return false;
				}
				break;
			case GML::Utils::AttributeList::BOOLEAN:
				if (config.AddBool(link->Name,(*(bool *)link->LocalAddress),link->MetaData)==false)
				{
					DEBUGMSG("Error adding : %s ",link->Name);
					return false;
				}
				break;
			case GML::Utils::AttributeList::UINT32:
				if (ListIndexToAttribute(link,(Int64)(*(UInt32 *)link->LocalAddress),&listValue))
				{
					if (config.AddString(link->Name,listValue.GetText(),link->MetaData)==false)
					{
						DEBUGMSG("Error adding : %s ",link->Name);
						return false;
					}
					break;
				}
				if (config.AddUInt32(link->Name,(*(UInt32 *)link->LocalAddress),link->MetaData)==false)
				{
					DEBUGMSG("Error adding : %s ",link->Name);
					return false;
				}
				break;
			case GML::Utils::AttributeList::INT32:
				if (ListIndexToAttribute(link,(Int64)(*(Int32 *)link->LocalAddress),&listValue))
				{
					if (config.AddString(link->Name,listValue.GetText(),link->MetaData)==false)
					{
						DEBUGMSG("Error adding : %s ",link->Name);
						return false;
					}
					break;
				}
				if (config.AddInt32(link->Name,(*(Int32 *)link->LocalAddress),link->MetaData)==false)
				{
					DEBUGMSG("Error adding : %s ",link->Name);
					return false;
				}
				break;
			default:				
				DEBUGMSG("Don`t know how to add : %s ",link->Name);
				return false;
		}
	}
	return true;
}
bool GML::Utils::GMLObject::SavePropery(char *fileName)
{
	GML::Utils::AttributeList	config;
	if (GetProperty(config)==false)
		return false;
	return config.Save(fileName);
}
char*GML::Utils::GMLObject::GetObjectName()
{
	return ObjectName;
}
