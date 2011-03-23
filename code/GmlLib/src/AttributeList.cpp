#include "AttributeList.h"

static unsigned int AttributeSizes[]={1,1,2,4,8,1,2,4,8,4,8,0};
static char *AttributeTypeName[]={"BOOL","INT8","INT16","INT32","INT64","UINT8","UINT16","UINT32","UINT64","FLOAT","DOUBLE","STRING"};


int  AttributeCompare(GML::Utils::Attribute &a1,GML::Utils::Attribute &a2,void *context)
{
	return GML::Utils::GString::Compare(a1.Name,a2.Name,true);
}
void ClearAttribute(GML::Utils::Attribute *a)
{
	if (!a) return;
	if (a->Data!=NULL) 
		delete a->Data;
	if (a->Name!=NULL)
		delete a->Name;
	if (a->MetaData==NULL)
		delete a->MetaData;
	MEMSET(a,0,sizeof(GML::Utils::Attribute));
}
bool AttributeToStringForm(GML::Utils::Attribute *a,GML::Utils::GString *tmp)
{
	tmp->Set("");
	if ((a->MetaData!=NULL) && (tmp->AddFormated("[%s]\n",a->MetaData)==false))
		return false;
	if (tmp->AddFormated("%s = ",a->Name)==false)
		return false;
	switch (a->AttributeType)
	{
		case GML::Utils::AttributeList::BOOLEAN	:
		case GML::Utils::AttributeList::INT8	:
		case GML::Utils::AttributeList::INT16	:
		case GML::Utils::AttributeList::INT32	:
		case GML::Utils::AttributeList::INT64	:
		case GML::Utils::AttributeList::UINT8	:
		case GML::Utils::AttributeList::UINT16	:
		case GML::Utils::AttributeList::UINT32	:
		case GML::Utils::AttributeList::UINT64	:
		case GML::Utils::AttributeList::FLOAT	:
		case GML::Utils::AttributeList::DOUBLE	:
			if (tmp->AddFormated("(%s) ",AttributeTypeName[a->AttributeType])==false) return false;
			break;		
		case GML::Utils::AttributeList::STRING	:			
			break;
		default:
			return false;
	}
	if (a->ElementsCount>1)
	{
		if (tmp->Add("[")==false)
			return false;
	}
	for (unsigned int tr=0;tr<a->ElementsCount;tr++)
	{
		switch (a->AttributeType)
		{
			case GML::Utils::AttributeList::BOOLEAN	:
				if (((bool *)a->Data)[tr])
				{
					if (tmp->Add("True")==false) return false;
				} else {
					if (tmp->Add("False")==false) return false;
				}					
				break;
			case GML::Utils::AttributeList::INT8	:
				if (tmp->AddFormated("%d",((Int8 *)a->Data)[tr])==false) return false;
				break;
			case GML::Utils::AttributeList::INT16	:
				if (tmp->AddFormated("%d",((Int16 *)a->Data)[tr])==false) return false;
				break;
			case GML::Utils::AttributeList::INT32	:
				if (tmp->AddFormated("%d",((Int32 *)a->Data)[tr])==false) return false;
				break;
			case GML::Utils::AttributeList::INT64	:
				if (tmp->AddFormated("0x%08X%08X",((Int64 *)a->Data)[tr])==false) return false;
				break;
			case GML::Utils::AttributeList::UINT8	:
				if (tmp->AddFormated("%u",((UInt8 *)a->Data)[tr])==false) return false;
				break;
			case GML::Utils::AttributeList::UINT16	:
				if (tmp->AddFormated("%u",((UInt16 *)a->Data)[tr])==false) return false;
				break;
			case GML::Utils::AttributeList::UINT32	:
				if (tmp->AddFormated("%u",((UInt32 *)a->Data)[tr])==false) return false;
				break;
			case GML::Utils::AttributeList::UINT64	:
				if (tmp->AddFormated("0x%08X%08X",((UInt64 *)a->Data)[tr])==false) return false;
				break;
			case GML::Utils::AttributeList::FLOAT	:
				if (tmp->AddFormated("%f",(double)((float *)a->Data)[tr])==false) return false;
				break;
			case GML::Utils::AttributeList::DOUBLE	:
				if (tmp->AddFormated("%f",((double *)a->Data)[tr])==false) return false;
				break;
			case GML::Utils::AttributeList::STRING	:
				if (tmp->AddFormated("\"%s\"",(char *)a->Data)==false) return false;
				break;
			default:
				return false;
		}
		if (tr+1<a->ElementsCount)
		{
			if (tmp->Add(",")==false)
				return false;
		}
	}
	if (a->ElementsCount>1)
	{
		if (tmp->Add("]")==false)
			return false;
	}
	return (tmp->Add("\n\n"));
}
bool AttributeProcessType(GML::Utils::GString *value,unsigned int *type,unsigned int *elementsCount)
{
	GML::Utils::GString		tmp;
	Int64					int64Value;
	Int32					int32Value;
	UInt64					uint64Value;
	UInt32					uint32Value;
	double					doubleValue;
	
	// stringuri
	if ((value->StartsWith("\"")) || (value->StartsWith("'")))
	{
		value->Truncate(value->Len()-1);
		if (value->Delete(0,1)==false)
			return false;
		(*type) = GML::Utils::AttributeList::STRING;
		(*elementsCount) = 1;
		return true;
	}
	// tipuri predefinite
	for (int tr=0;tr<GML::Utils::AttributeList::ATTRIBUTES_COUNT;tr++)
	{
		if (tmp.SetFormated("(%s)",AttributeTypeName[tr])==false)
			return false;
		if (value->StartsWith(tmp,true))
		{
			(*type)=tr;
			(*elementsCount) = 1;
			if (value->Delete(0,tmp.Len())==false)
				return false;
			if (value->Strip()==false)
				return false;
			if ((value->StartsWith("[")) && (value->EndsWith("]")))
			{
				value->Truncate(value->Len()-1);
				if (value->Delete(0,1)==false)
					return false;
				for (int gr=0;gr<value->Len();gr++)
					if (value->GetText()[gr]==',')
						(*elementsCount)++;				
			}
			return true;
		}
	}
	// boolean
	if ((value->Equals("True",true)) || (value->Equals("False",true)) || (value->Equals("Yes",true)) || (value->Equals("No",true)))
	{
		(*type) = GML::Utils::AttributeList::BOOLEAN;
		(*elementsCount) = 1;
		return true;
	}
	// verific daca e un numar negativ
	if (value->StartsWith("-"))
	{
		if (value->ConvertToInt64(&int64Value))
		{
			if ((value->ConvertToInt32(&int32Value)) && (int32Value==(Int32)int64Value))
			{
				(*type) = GML::Utils::AttributeList::INT32;
				(*elementsCount) = 1;
				return true;
			}
			(*type) = GML::Utils::AttributeList::INT64;
			(*elementsCount) = 1;
			return true;
		}
	}
	// verific daca nu e un numar pozitiv
	if (value->ConvertToUInt64(&uint64Value))
	{
		if ((value->ConvertToUInt32(&uint32Value)) && (uint32Value==(UInt32)uint64Value))
		{
			(*type) = GML::Utils::AttributeList::UINT32;
			(*elementsCount) = 1;
			return true;
		}
		(*type) = GML::Utils::AttributeList::UINT64;
		(*elementsCount) = 1;
		return true;
	}
	// verific daca e un double
	if (value->ConvertToDouble(&doubleValue))
	{
		(*type) = GML::Utils::AttributeList::DOUBLE;
		(*elementsCount) = 1;
		return true;
	}

	// daca nu e nici unul dintre acestea , presupun ca este un string
	(*type) = GML::Utils::AttributeList::STRING;
	(*elementsCount) = 1;
	return true;
}
bool AttributeProcessValues(GML::Utils::GString *value,unsigned char *Data,unsigned int elemensCount,unsigned int type)
{
	GML::Utils::GString			tok;
	int				pos = 0;
	unsigned int	index=0;
	
	if (type==GML::Utils::AttributeList::STRING)
	{
		MEMCOPY(Data,value->GetText(),value->Len());
		Data[value->Len()]=0;
		return true;
	}

	while ((value->CopyNext(&tok,",",&pos)) && (index<elemensCount))
	{
		switch (type)
		{
			case GML::Utils::AttributeList::BOOLEAN:
				if ((tok.Equals("Yes",true)) || (tok.Equals("True",true)))
					((bool *)Data)[index] = true;
				else
					((bool *)Data)[index] = false;
				break;
			case GML::Utils::AttributeList::INT8:
				if (tok.ConvertToInt8(&(((Int8 *)Data)[index]))==false)
					return false;
				break;
			case GML::Utils::AttributeList::INT16:
				if (tok.ConvertToInt16(&(((Int16 *)Data)[index]))==false)
					return false;
				break;
			case GML::Utils::AttributeList::INT32:
				if (tok.ConvertToInt32(&(((Int32 *)Data)[index]))==false)
					return false;
				break;
			case GML::Utils::AttributeList::INT64:
				if (tok.ConvertToInt64(&(((Int64 *)Data)[index]))==false)
					return false;
				break;
			case GML::Utils::AttributeList::UINT8:
				if (tok.ConvertToUInt8(&(((UInt8 *)Data)[index]))==false)
					return false;
				break;
			case GML::Utils::AttributeList::UINT16:
				if (tok.ConvertToUInt16(&(((UInt16 *)Data)[index]))==false)
					return false;
				break;
			case GML::Utils::AttributeList::UINT32:
				if (tok.ConvertToUInt32(&(((UInt32 *)Data)[index]))==false)
					return false;
				break;
			case GML::Utils::AttributeList::UINT64:
				if (tok.ConvertToUInt64(&(((UInt64 *)Data)[index]))==false)
					return false;
				break;
			case GML::Utils::AttributeList::FLOAT:
				if (tok.ConvertToFloat(&(((float *)Data)[index]))==false)
					return false;
				break;
			case GML::Utils::AttributeList::DOUBLE:
				if (tok.ConvertToDouble(&(((double *)Data)[index]))==false)
					return false;
				break;
			default:
				return false;
		}
		index++;
	}
	return (index == elemensCount);
}
bool GML::Utils::Attribute::operator< (GML::Utils::Attribute &a)
{
	return (bool)(GString::Compare(Name,a.Name)<0);
}
bool GML::Utils::Attribute::operator> (GML::Utils::Attribute &a)
{
	return (bool)(GString::Compare(Name,a.Name)>0);
}
bool GML::Utils::Attribute::GetListItem(GML::Utils::GString &str)
{
	GML::Utils::GString		temp,line,word;
	int						poz=0,pw;

	if (MetaData==NULL)
		return true;
	if (str.Set("")==false)
		return false;
	if (temp.Set(MetaData)==false)
		return false;
	while (temp.CopyNextLine(&line,&poz))
	{
		if ((line.StartsWith("!!LIST:")) && (line.EndsWith("!!")))
		{
			if (line.Replace("!!LIST:","")==false)
				return false;
			line.Truncate(line.Len()-2);
			pw = 0;
			while (line.CopyNext(&word,",",&pw))
			{
				if (word.Contains("="))
					word.Truncate(word.Find("="));
				if (str.Add(&word)==false)
					return false;
				if (str.Add(",")==false)
					return false;
			}
			if (str.EndsWith(","))
				str.Truncate(str.Len()-1);
			return true;
		} 
	}
	return true;
}
bool GML::Utils::Attribute::GetDescription(GML::Utils::GString &str)
{
	GML::Utils::GString		temp,line,word;
	int						poz=0;

	if (MetaData==NULL)
		return true;
	if (str.Set("")==false)
		return false;
	if (temp.Set(MetaData)==false)
		return false;
	while (temp.CopyNextLine(&line,&poz))
	{
		if (line.StartsWith("!!"))
			continue;
		if (str.Add(&line)==false)
			return false;
		if (str.Add("\n")==false)
			return false;
	}
	if (str.Strip()==false)
		return false;
	return true;
}
//--------------------------------------------------------------------------------------
GML::Utils::AttributeList::AttributeList(void)
{

}

GML::Utils::AttributeList::~AttributeList(void)
{
	Clear();
}
void GML::Utils::AttributeList::Clear()
{
	GML::Utils::Attribute *a;
	for (unsigned int tr=0;tr<list.Len();tr++)
	{
		a = (GML::Utils::Attribute *)list.GetPtrToObject(tr);
		ClearAttribute(a);
	}
	list.DeleteAll();
}
bool GML::Utils::AttributeList::Update(char *Name,void *Data,UInt32 DataSize)
{
	GML::Utils::Attribute		*a;

	if ((Data==NULL) || ((a=Get(Name))==NULL))
		return false;
	if (a->Data==NULL)
		return false;
	if (DataSize!=a->DataSize)
		return false;
	MEMCOPY(Data,a->Data,a->DataSize);
	return true;
}
bool GML::Utils::AttributeList::UpdateString(char *Name,GML::Utils::GString &text, bool useDefault,char* defaultValue)
{
	GML::Utils::Attribute		*a;

	if ((a=Get(Name))==NULL)
	{
		if (useDefault)
			return text.Set(defaultValue);
		else
			return false;
	}
	if (a->Data==NULL)
		return false;
	return text.Set((char*)a->Data,a->DataSize);
}
bool GML::Utils::AttributeList::UpdateBool(char *Name,bool &boolValue,bool useDefault,bool defaultValue)
{
	GML::Utils::Attribute		*a;

	if ((a=Get(Name))==NULL)
	{
		if (useDefault)
		{
			boolValue = defaultValue;
			return true;
		}
		return false;
	}
	if (a->DataSize!=sizeof(bool))
		return false;
	if (a->Data==NULL)
		return false;
	boolValue = *(bool *)a->Data;
	return true;
}
bool GML::Utils::AttributeList::UpdateUInt32(char *Name,UInt32 &uint32Value,bool useDefault,UInt32 defaultValue)
{
	GML::Utils::Attribute		*a;

	if ((a=Get(Name))==NULL)
	{
		if (useDefault)
		{
			uint32Value = defaultValue;
			return true;
		}
		return false;
	}
	if (a->DataSize!=sizeof(UInt32))
		return false;
	if (a->Data==NULL)
		return false;
	uint32Value = *(UInt32 *)a->Data;
	return true;
}
bool GML::Utils::AttributeList::UpdateInt32(char *Name,Int32 &int32Value,bool useDefault,Int32 defaultValue)
{
	GML::Utils::Attribute		*a;

	if ((a=Get(Name))==NULL)
	{
		if (useDefault)
		{
			int32Value = defaultValue;
			return true;
		}
		return false;
	}
	if (a->DataSize!=sizeof(Int32))
		return false;
	if (a->Data==NULL)
		return false;
	int32Value = *(Int32 *)a->Data;
	return true;
}
bool GML::Utils::AttributeList::UpdateDouble(char *Name,double &doubleValue,bool useDefault,double defaultValue)
{
	GML::Utils::Attribute		*a;

	if ((a=Get(Name))==NULL)
	{
		if (useDefault)
		{
			doubleValue = defaultValue;
			return true;
		}
		return false;
	}
	if (a->DataSize!=sizeof(double))
		return false;
	if (a->Data==NULL)
		return false;
	doubleValue = *(double *)a->Data;
	return true;
}

bool GML::Utils::AttributeList::AddAttribute(char* Name,void *Data,unsigned int AttributeType,unsigned int ElementsCount,char *Description)
{
	GML::Utils::Attribute		a;
	unsigned int				NameLen,DescLen;
	
	MEMSET(&a,0,sizeof(a));

	if ((Name==NULL) || (Data==NULL) || (AttributeType>=GML::Utils::AttributeList::ATTRIBUTES_COUNT) || (ElementsCount<1))
		return false;
	if ((AttributeType==GML::Utils::AttributeList::STRING) && (ElementsCount!=1))
		return false;
	a.AttributeType = AttributeType;
	a.ElementsCount = ElementsCount;
	
	if (AttributeType==GML::Utils::AttributeList::STRING)
		a.DataSize = GML::Utils::GString::Len((char*)Data)+1;
	else
		a.DataSize = AttributeSizes[AttributeType]*ElementsCount;
	
	NameLen = GML::Utils::GString::Len(Name);
	if (Description==NULL)
		DescLen = 0;
	else
		DescLen = GML::Utils::GString::Len(Description);

	while (true)
	{
		if ((a.Data = new unsigned char [a.DataSize])==NULL)
			break;
		if ((a.Name = new char [NameLen+1])==NULL)
			break;
		if (Description)
			if ((a.MetaData = new char [DescLen+1])==NULL)
				break;
		MEMCOPY(a.Name,Name,NameLen+1);
		MEMCOPY(a.Data,Data,a.DataSize);
		if (Description)
			MEMCOPY(a.MetaData,Description,DescLen+1);
		if (list.PushByRef(a)==false)
			break;
		list.Sort(true,AttributeCompare);
		return true;
	}
	ClearAttribute(&a);
	return false;
}
bool GML::Utils::AttributeList::AddString(char *Name, char *Text, char *Description)
{
	return AddAttribute(Name,Text,STRING,1,Description);
}
bool GML::Utils::AttributeList::AddBool(char *Name, bool value,char *Description)
{
	return AddAttribute(Name,&value,BOOLEAN,1,Description);
}
bool GML::Utils::AttributeList::AddDouble(char *Name, double value, char *Description)
{
	return AddAttribute(Name,&value,DOUBLE,1,Description);
}
bool GML::Utils::AttributeList::AddUInt32(char *Name, UInt32 value, char *Description)
{	
	return AddAttribute(Name,&value,UINT32,1,Description);
}
bool GML::Utils::AttributeList::AddInt32(char *Name, Int32 value, char *Description)
{
	return AddAttribute(Name,&value,INT32,1,Description);
}
GML::Utils::Attribute* GML::Utils::AttributeList::Get(unsigned int index)
{
	return (GML::Utils::Attribute *)list.GetPtrToObject(index);
}
GML::Utils::Attribute* GML::Utils::AttributeList::Get(char *Name)
{
	GML::Utils::Attribute	a;
	int			index;

	a.Name = Name;
	if (list.BinarySearch(a,&index,NULL,AttributeCompare))
	{
		if (index>=0)
			return Get((unsigned int)index);
	}
	return NULL;
}
unsigned int GML::Utils::AttributeList::GetCount()
{
	return list.Len();
}
bool GML::Utils::AttributeList::Save(char *fileName)
{
	GML::Utils::File			f;
	unsigned int				tr,count;
	GML::Utils::Attribute		*a;
	GML::Utils::GString			temp;

	if (temp.Create(2048)==false)
		return false;
	if (f.Create(fileName)==false)
		return false;
	count = list.Len();
	
	for (tr=0;tr<count;tr++)
		if ((a=Get(tr))!=NULL)
		{
			if (AttributeToStringForm(a,&temp)==false)
			{
				f.Close();
				temp.Distroy();
				return false;
			}
			if (f.Write(temp.GetText(),temp.GetSize())==false)
			{
				f.Close();
				temp.Distroy();
				return false;
			}
		}

	f.Close();
	return true;
}
bool GML::Utils::AttributeList::FromString(GML::Utils::GString &text)
{
	GML::Utils::GString			line,desc,name,value,Buffer;
	int							index=0,eq_poz,tr;
	unsigned int				Type,ElementsCount,Size;
	bool						onDesc=false;

	if (line.Create(2048)==false)
		return false;
	if (Buffer.Create(2048)==false)
		return false;

	while (text.CopyNextLine(&line,&index))
	{
		if (line.Strip()==false)
			return false;
		if ((line.StartsWith("#")) || (line.StartsWith(";")))	// comentariu
			continue;
		if (line.Len()==0)
			continue;
		if ((line.StartsWith("[")) && (line.EndsWith("]")))
		{
			desc.Set(&line.GetText()[1],line.GetSize()-2);
			continue;
		}
		if (onDesc) 
		{
			desc.Add("\n");
			if (line.EndsWith("]"))
			{
				desc.Add(line.GetText(),line.Len()-1);
				onDesc = false;
			} else {
				desc.Add(&line);
			}
			continue;
		}
		if ((line.StartsWith("[")) && (line.EndsWith("]")==false))
		{
			desc.Set(&line.GetText()[1],line.GetSize()-1);
			onDesc = true;
			continue;
		}
		eq_poz = line.Find("=");
		if ((line.StartsWith("[")==false) && (eq_poz>0))
		{
			// cod normal
			if (name.Set(line.GetText(),eq_poz)==false)
				return false;
			if (value.Set(&line.GetText()[eq_poz+1])==false)
				return false;
			if (name.Strip()==false)
				return false;
			if (value.Strip()==false)
				return false;
			// verific ca numele sa fie valid
			for (tr=0;tr<name.Len();tr++)
			{
				TCHAR	ch = name.GetText()[tr];
				if (((ch>='A') && (ch<='Z')) || ((ch>='a') && (ch<='z')) || ((ch>='0') && (ch<='9')) || (ch=='_'))
				{
				} else {
					return false; // caracter invalid in nume
				}
			}
			// numele e ok , incerc sa creez bufferul
			Type = 0xFFFFFFFF;
			ElementsCount = 1;
			if (AttributeProcessType(&value,&Type,&ElementsCount)==false)
				return false;
			if ((ElementsCount<1) || (Type>=ATTRIBUTES_COUNT))
				return false;
			if (Type==GML::Utils::AttributeList::STRING)
				Size = value.Len()+1;
			else
				Size = ElementsCount * AttributeSizes[Type];
			if (Size==0)
				return false;
			// aloc daca e cazul
			if (Buffer.Set("")==false)
				return false;
			if (Buffer.Grow(Size)==false)
				return false;
			if (AttributeProcessValues(&value,(unsigned char *)Buffer.GetText(),ElementsCount,Type)==false)
				return false;
			if (desc.Len()>0)
			{
				if (AddAttribute(name.GetText(),Buffer.GetText(),Type,ElementsCount,desc.GetText())==false)
					return false;
			} else {
				if (AddAttribute(name.GetText(),Buffer.GetText(),Type,ElementsCount)==false)
					return false;
			}
			desc.Set("");
			continue;
		}
		return false;
	}
	list.Sort(true,AttributeCompare);

	return true;
}
bool GML::Utils::AttributeList::Load(char *fileName)
{
	GML::Utils::GString			temp;

	Clear();

	if (temp.LoadFromFile(fileName)==false)
		return false;
	if (FromString(temp)==false)
		return false;

	return true;
}
bool GML::Utils::AttributeList::Create(char *text,char separator)
{
	GML::Utils::GString			temp;
	char*						txt;
	int							tr,count;

	Clear();

	if (text==NULL)
		return false;
	if (temp.Set(text)==false)
		return false;
	if ((separator!=0) && (separator!='\n'))
	{
		txt = temp.GetText();
		count=0;
		for (tr=0;txt[tr]!=0;tr++)
		{
			if ((txt[tr]==separator) && (count==0))
				txt[tr]='\n';
			if (txt[tr]=='{')
				count++;
			if (txt[tr]=='}')
				count--;
		}
	}

	return FromString(temp);
}