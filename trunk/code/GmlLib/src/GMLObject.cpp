#include "GMLObject.h"

#define DEBUGMSG	printf

GML::Utils::GMLObject::GMLObject()
{
	Description = "";
	Author = "";
	AttrLinks.DeleteAll();
}
bool GML::Utils::GMLObject::LinkPropertyToString(char *Name,GML::Utils::GString &LocalAddr,char *defaultValue,char *Description)
{
	GML::Utils::AttributeLink	link;

	if (LocalAddr.Set(defaultValue)==false)
		return false;
	link.Description = Description;
	link.Name = Name;
	link.AttributeType = GML::Utils::AttributeList::STRING;
	link.LocalAddress = &LocalAddr;

	return AttrLinks.PushByRef(link);
}
bool GML::Utils::GMLObject::LinkPropertyToBool(char *Name,bool &LocalAddr,bool defaultValue,char *Description)
{
	GML::Utils::AttributeLink	link;

	LocalAddr = defaultValue;
	link.Description = Description;
	link.Name = Name;
	link.AttributeType = GML::Utils::AttributeList::BOOLEAN;
	link.LocalAddress = &LocalAddr;

	return AttrLinks.PushByRef(link);
}
bool GML::Utils::GMLObject::LinkPropertyToDouble(char *Name,double &LocalAddr,double defaultValue,char *Description)
{
	GML::Utils::AttributeLink	link;

	LocalAddr = defaultValue;
	link.Description = Description;
	link.Name = Name;
	link.AttributeType = GML::Utils::AttributeList::DOUBLE;
	link.LocalAddress = &LocalAddr;

	return AttrLinks.PushByRef(link);
}
bool GML::Utils::GMLObject::LinkPropertyToUInt32(char *Name,UInt32 &LocalAddr,UInt32 defaultValue,char *Description)
{
	GML::Utils::AttributeLink	link;

	LocalAddr = defaultValue;
	link.Description = Description;
	link.Name = Name;
	link.AttributeType = GML::Utils::AttributeList::UINT32;
	link.LocalAddress = &LocalAddr;

	return AttrLinks.PushByRef(link);
}
bool GML::Utils::GMLObject::LinkPropertyToInt32(char *Name,Int32 &LocalAddr,Int32 defaultValue,char *Description)
{
	GML::Utils::AttributeLink	link;

	LocalAddr = defaultValue;
	link.Description = Description;
	link.Name = Name;
	link.AttributeType = GML::Utils::AttributeList::INT32;
	link.LocalAddress = &LocalAddr;

	return AttrLinks.PushByRef(link);
}
bool GML::Utils::GMLObject::SetProperty(GML::Utils::AttributeList &config)
{
	UInt32						tr;
	GML::Utils::AttributeLink	*link;
	GML::Utils::Attribute		*attr;

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
				if (attr->AttributeType!=GML::Utils::AttributeList::BOOLEAN)
				{					
					DEBUGMSG("Invalid Attribute type (expecting boolean) for %s ",link->Name);
					return false;
				}
				(*((bool *)link->LocalAddress)) = *((bool *)attr->Data);
				break;
			case GML::Utils::AttributeList::DOUBLE:
				if (attr->AttributeType==GML::Utils::AttributeList::DOUBLE) 
				{
					(*((double *)link->LocalAddress)) = *((double *)attr->Data);
					break;
				}
				if (attr->AttributeType==GML::Utils::AttributeList::UINT32) 
				{
					(*((double *)link->LocalAddress)) = (double)(*((UInt32 *)attr->Data));
					break;
				}
				if (attr->AttributeType==GML::Utils::AttributeList::INT32) 
				{
					(*((double *)link->LocalAddress)) = (double)(*((Int32 *)attr->Data));
					break;
				}				
				DEBUGMSG("Expecting a numerical value (double,uint32,int32) for %s ",link->Name);
				return false;
			case GML::Utils::AttributeList::UINT32:
				if (attr->AttributeType==GML::Utils::AttributeList::UINT32) 
				{
					(*((UInt32 *)link->LocalAddress)) = *((UInt32 *)attr->Data);
					break;
				}
				if (attr->AttributeType==GML::Utils::AttributeList::INT32) 
				{
					(*((UInt32 *)link->LocalAddress)) = (UInt32)(*((Int32 *)attr->Data));
					break;
				}				
				DEBUGMSG("Expecting a numerical value (uint32,int32) for %s ",link->Name);
				return false;
			case GML::Utils::AttributeList::INT32:
				if (attr->AttributeType==GML::Utils::AttributeList::INT32) 
				{
					(*((Int32 *)link->LocalAddress)) = *((Int32 *)attr->Data);
					break;
				}
				if (attr->AttributeType==GML::Utils::AttributeList::UINT32) 
				{
					(*((Int32 *)link->LocalAddress)) = (Int32)(*((UInt32 *)attr->Data));
					break;
				}				
				DEBUGMSG("Expecting a numerical value (uint32,int32) for %s ",link->Name);
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
bool GML::Utils::GMLObject::GetProperty(GML::Utils::AttributeList &config)
{
	UInt32						tr;
	GML::Utils::AttributeLink	*link;
	
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
				if (config.AddString(link->Name,(*(GML::Utils::GString *)link->LocalAddress).GetText(),link->Description)==false)
					DEBUGMSG("Error adding : %s ",link->Name);
				break;
			case GML::Utils::AttributeList::DOUBLE:
				if (config.AddDouble(link->Name,(*(double *)link->LocalAddress),link->Description)==false)
					DEBUGMSG("Error adding : %s ",link->Name);
				break;
			case GML::Utils::AttributeList::BOOLEAN:
				if (config.AddBool(link->Name,(*(bool *)link->LocalAddress),link->Description)==false)
					DEBUGMSG("Error adding : %s ",link->Name);
				break;
			case GML::Utils::AttributeList::UINT32:
				if (config.AddUInt32(link->Name,(*(UInt32 *)link->LocalAddress),link->Description)==false)
					DEBUGMSG("Error adding : %s ",link->Name);
				break;
			case GML::Utils::AttributeList::INT32:
				if (config.AddInt32(link->Name,(*(Int32 *)link->LocalAddress),link->Description)==false)
					DEBUGMSG("Error adding : %s ",link->Name);
				break;
			default:				
				DEBUGMSG("Don`t know how to add : %s ",link->Name);
				return false;
		}
	}
	return true;
}