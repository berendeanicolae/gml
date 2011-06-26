#include "GString.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static char *convert_array_upper={"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"};
static char *convert_array_lower={"0123456789abcdefghijklmnopqrstuvwxyz"};

#define GSTRING_TEST_AND_INIT { if (Text==NULL) { if (Create(64)==false) return false; } }
#define GSTRING_GROW_WITH(x) { 	if (x+Size+1>=MaxSize) {	if (Grow(x+Size+32)==false)	return false; }  }
#define LOWER_CHAR(x) (((x>='A') && (x<='Z'))?x|0x20:x)
#define CONVERT_TO_COMPARE(x,ignore) (((ignore) && (x>='A') && (x<='Z'))?x|0x20:x)
#define MEMMOVE	memmove
#define MEMCOPY	memcpy
#define MAX_FORMATED_EX_TEMP_BUF	512
#define MAX_EXTVALUE				32

struct DataConvertInfo
{
	enum {
		TYPE_None = 0,
		TYPE_Bool,
		TYPE_Char,
		TYPE_WChar,
		TYPE_UInt8,
		TYPE_UInt16,
		TYPE_UInt32,
		TYPE_UInt64,
		TYPE_Int8,
		TYPE_Int16,
		TYPE_Int32,
		TYPE_Int64,
		TYPE_Ascii,
		TYPE_Unicode,
		TYPE_Float,
		TYPE_Double,		
	};
	enum {
		ALIGN_LEFT,
		ALIGN_RIGHT,
		ALIGN_CENTER
	};
	enum {
		FLAG_UPPER = 1,
		FLAG_TRUNCATE = 2,
	};
	enum {
		EXTVALUE_AlignSize = 0,
		EXTVALUE_Base,
		EXTVALUE_Zecimals,
		EXTVALUE_GROUP,
		EXTVALUE_FillChar
	};
	unsigned int	DataType;
	unsigned int	AlignSize;
	unsigned char	Align;
	unsigned int	Base;
	unsigned int	Flags;
	unsigned int	Zecimals;
	unsigned int	Group;
	char			FillChar;
	unsigned int	ExternalValue[MAX_EXTVALUE];
	unsigned int	ExternalValuesCount;
};
union DataValueUnion
{
	unsigned char	vUInt8;
	unsigned short	vUInt16;
	unsigned int	vUInt32;
	TYPE_UINT64		vUInt64;

	char			vInt8;
	short			vInt16;
	int				vInt32;
	TYPE_UINT64		vInt64;

	char			vChar;
	short			vWChar;

	char*			vAscii;
	bool			vBool;

	float			vFloat;
	double			vDouble;
};
// tipuri
struct StructTypeName
{
	char			*Name;
	unsigned int	Id;
};
static StructTypeName __TypeNames[] = {
	{"b",			DataConvertInfo::TYPE_Bool},
	{"bool",		DataConvertInfo::TYPE_Bool},

	{"i8",			DataConvertInfo::TYPE_Int8},
	{"int8",		DataConvertInfo::TYPE_Int8},
	{"char",		DataConvertInfo::TYPE_Int8},

	{"c",			DataConvertInfo::TYPE_Char},
	
	{"uc",			DataConvertInfo::TYPE_WChar},
	{"wchar",		DataConvertInfo::TYPE_WChar},

	{"i16",			DataConvertInfo::TYPE_Int16},
	{"int16",		DataConvertInfo::TYPE_Int16},
	{"short",		DataConvertInfo::TYPE_Int16},

	{"i32",			DataConvertInfo::TYPE_Int32},
	{"int32",		DataConvertInfo::TYPE_Int32},
	{"int",			DataConvertInfo::TYPE_Int32},

	{"i64",			DataConvertInfo::TYPE_Int64},
	{"int64",		DataConvertInfo::TYPE_Int64},

	{"ui8",			DataConvertInfo::TYPE_UInt8},
	{"uint8",		DataConvertInfo::TYPE_UInt8},
	{"byte",		DataConvertInfo::TYPE_UInt8},

	{"ui16",		DataConvertInfo::TYPE_UInt16},
	{"uint16",		DataConvertInfo::TYPE_UInt16},
	{"w",			DataConvertInfo::TYPE_UInt16},

	{"ui32",		DataConvertInfo::TYPE_UInt32},
	{"uint32",		DataConvertInfo::TYPE_UInt32},
	{"uint",		DataConvertInfo::TYPE_UInt32},
	{"dw",			DataConvertInfo::TYPE_UInt32},

	{"ui64",		DataConvertInfo::TYPE_UInt64},
	{"uint64",		DataConvertInfo::TYPE_UInt64},
	{"qw",			DataConvertInfo::TYPE_UInt64},

	{"s",			DataConvertInfo::TYPE_Ascii},
	{"str",			DataConvertInfo::TYPE_Ascii},
	{"string",		DataConvertInfo::TYPE_Ascii},
	{"a",			DataConvertInfo::TYPE_Ascii},
	{"ascii",		DataConvertInfo::TYPE_Ascii},

	{"u",			DataConvertInfo::TYPE_Unicode},
	{"unicode",		DataConvertInfo::TYPE_Unicode},

	{"f",			DataConvertInfo::TYPE_Float},
	{"float",		DataConvertInfo::TYPE_Float},

	{"r",			DataConvertInfo::TYPE_Double},
	{"dbl",			DataConvertInfo::TYPE_Double},
	{"double",		DataConvertInfo::TYPE_Double},

};


bool Simple_IsCharMatching(TCHAR ct,TCHAR cm,bool IgnoreCase)
{
	if (IgnoreCase)
	{
		if ((ct>='A') && (ct<='Z')) ct+=32;
		if ((cm>='A') && (cm<='Z')) cm+=32;
	}
	if (cm=='?') cm=ct;
	return (ct==cm);
}
int  Simple_MatchFindNext(TCHAR *text,TCHAR car,bool IgnoreCase)
{
	int			tr;

	if (car==0) return -2;

	for (tr=0;text[tr]!=0;tr++)
		if (Simple_IsCharMatching(text[tr],car,IgnoreCase)) 
			return tr;
	return -1;
}
bool Simple_Match(TCHAR *text,TCHAR *masca,bool IgnoreCase,bool bStartWith)
{
	int		tr,found,tpoz;
	TCHAR	ct,cm;

	for (tr=0;(text[tr]!=0) && (masca[tr]!=0);tr++)
	{
		ct=text[tr];
		cm=masca[tr];
		if (cm=='*')
		{
			tpoz=tr;
			do
			{
				tpoz++;
				found=Simple_MatchFindNext(&text[tpoz],masca[tr+1],IgnoreCase);
				if (found==-2) 
					return true;
				if (found<0) 
					return false;
				tpoz+=found;
			} 
			while ((text[tpoz]!=0) && (Simple_Match(&text[tpoz],&masca[tr+1],IgnoreCase,bStartWith)==false));
			return (text[tpoz]!=0);
		} else {
			if (Simple_IsCharMatching(ct,cm,IgnoreCase)==false) 
				return false;
		}
	}
	if (bStartWith)
	{
		return (masca[tr]==0);
	} else {
		return ((text[tr]==masca[tr]) && (text[tr]==0));
	}
}


bool StringToNumber(TCHAR *text,unsigned int len,TYPE_UINT64 *value,unsigned int base)
{
	TYPE_UINT64			cVal;
	unsigned int		tr;

	if ((text==NULL) || (value==NULL) || (base<2) || (base>16))
		return false;
	(*value)=0;
	for (tr=0;(tr<len) && ((*text)!=0);tr++,text++)
	{
		if (((*text)>='0') && ((*text)<='9'))
			cVal = (*text)-'0';
		else if (((*text)>='A') && ((*text)<='Z'))
				cVal = (*text)-'A'+10;
		else if (((*text)>='a') && ((*text)<='z'))
				cVal = (*text)-'a'+10;
		else return false;
		if (cVal>=base)
			return false;
		(*value) = (*value)*base+cVal;
	}
	return true;
}
bool GetStringNumberInformations(TCHAR *text,unsigned int len,bool *isSigned,unsigned int *base,unsigned int *startValue,unsigned int *textLen)
{
	unsigned int start=0;

	if ((text==NULL) || (isSigned==NULL) || (base==NULL) || (len==0) || (startValue==NULL) || (textLen==NULL))
		return false;
	
	(*isSigned)=false;
	(*base)=10;

	if (text[0]=='-')
		(*isSigned)=true;
	if ((text[0]=='-') || (text[0]=='+'))
		start++;
	if ((start+2<len) && (text[start]=='0') && (LOWER_CHAR(text[start+1])=='x'))
	{
		(*base)=16;
		start+=2;
	} else
	if (text[len-1]=='h')
	{
		(*base)=16;
		len--;
	} else
	if (text[start]=='0')
	{
		(*base)=8;
		start++;
	}
	(*startValue)=start;
	(*textLen) = len;
	return true;
}
bool ConvertStringToNumber(TCHAR *text,TYPE_UINT64 *value,unsigned int base,int textSize,bool onlyUnsigned,bool *_isSigned=NULL)
{
	bool			isSigned=false;
	unsigned int	start=0,numberLen,_base;
	TYPE_UINT64		result;
	
	if ((value==NULL) || (text==NULL))
		return false;
	if (textSize==-1)
	{
		for (textSize=0;text[textSize]!=0;textSize++);
	}
	if (GetStringNumberInformations(text,textSize,&isSigned,&_base,&start,&numberLen)==false)
		return false;
	if (base==0)
		base = _base;
	if ((onlyUnsigned) && (isSigned))
		return false;
	if (StringToNumber(&text[start],numberLen,&result,base)==false)
		return false;
	(*value) = result;
	if (_isSigned)
		(*_isSigned) = isSigned;
	return true;
}

bool __StrEq(char *s1,int s1Size,char *s2,bool ignoreCase=false)
{
	int		tr;
	char	c1,c2;

	for (tr=0;(s2[tr]!=0) && (tr<s1Size);tr++)
	{
		c1=CONVERT_TO_COMPARE(s1[tr],ignoreCase);
		c2=CONVERT_TO_COMPARE(s2[tr],ignoreCase);
		if (c1!=c2)
			return false;
	}
	if ((s2[tr]==0) && (tr==s1Size))
		return true;
	return false;
}
bool __StrToUInt32(char *s1,int s1Size,unsigned int *value)
{
	(*value)=0;
	if (s1Size<1)
		return false;
	for (int tr=0;tr<s1Size;tr++)
	{
		if ((s1[tr]>='0') && (s1[tr]<='9'))
		{
			(*value) = (*value) * 10 + s1[tr]-'0';
		} else {
			return false;
		}
	}
	return true;
}
void ResetDataConvertInfo(DataConvertInfo *dci)
{
	if (dci==NULL)
		return;
	dci->Align = DataConvertInfo::ALIGN_LEFT;
	dci->Base = 10;
	dci->Flags = 0;
	dci->DataType = DataConvertInfo::TYPE_None;
	dci->Zecimals = 0;
	dci->AlignSize = 0;
	dci->Group = 0xFFFFFFFF;
	dci->FillChar = ' ';
	dci->ExternalValuesCount = 0;
}
bool UpdateDataConvertInfo(DataConvertInfo *dci,char *info,int infoSize)
{
	unsigned int	tr;
	
	if ((info==NULL) || (info[0]==0) || (dci==NULL))
		return false;
	// tipul
	if ((info[0]>='a') && (info[0]<='z'))
	{
		for (tr=0;tr<sizeof(__TypeNames)/sizeof(StructTypeName);tr++)
		{
			if (__StrEq(info,infoSize,__TypeNames[tr].Name,false))
			{
				dci->DataType = __TypeNames[tr].Id;
				return true;
			}
		}
	}
	// verific daca nu e o baza cunoscuta
	if (__StrEq(info,infoSize,"hex",true))
	{
		dci->Base = 16;
		if (info[0]=='H') 
			dci->Flags |= DataConvertInfo::FLAG_UPPER;
		return true;
	}
	if (__StrEq(info,infoSize,"oct",true))
	{
		dci->Base = 8;
		if (info[0]=='O') 
			dci->Flags |= DataConvertInfo::FLAG_UPPER;
		return true;
	}
	if (__StrEq(info,infoSize,"bin",true))
	{
		dci->Base = 2;
		if (info[0]=='B') 
			dci->Flags |= DataConvertInfo::FLAG_UPPER;
		return true;
	}
	if (__StrEq(info,infoSize,"dec",true))
	{
		dci->Base = 10;
		if (info[0]=='D') 
			dci->Flags |= DataConvertInfo::FLAG_UPPER;
		return true;
	}
	if ((info[0]|0x20)=='b')
	{
		if ((infoSize==3) && (info[1]=='%') && (info[2]=='%'))
		{
			dci->ExternalValue[dci->ExternalValuesCount++] = DataConvertInfo::EXTVALUE_Base;
			if (dci->ExternalValuesCount == MAX_EXTVALUE)
				return false;
		} else {
			if (__StrToUInt32(&info[1],infoSize-1,&dci->Base)==false)
				return false;
			if ((dci->Base<2) || (dci->Base>=36))
				return false;
		}
		if (info[0]=='B') 
			dci->Flags |= DataConvertInfo::FLAG_UPPER;
		return true;
	}
	// verific alignamentul
	if (info[0]=='L')
	{
		if ((infoSize==3) && (info[1]=='%') && (info[2]=='%'))
		{
			dci->ExternalValue[dci->ExternalValuesCount++] = DataConvertInfo::EXTVALUE_AlignSize;
			if (dci->ExternalValuesCount == MAX_EXTVALUE)
				return false;
		} else {
			if (info[1]!=0)
			{
				if (__StrToUInt32(&info[1],infoSize-1,&dci->AlignSize)==false)
					return false;
			}
		}
		dci->Align = DataConvertInfo::ALIGN_LEFT;		
		return true;
	}
	if (info[0]=='R')
	{
		if ((infoSize==3) && (info[1]=='%') && (info[2]=='%'))
		{
			dci->ExternalValue[dci->ExternalValuesCount++] = DataConvertInfo::EXTVALUE_AlignSize;
			if (dci->ExternalValuesCount == MAX_EXTVALUE)
				return false;
		} else {
			if (info[1]!=0)
			{
				if (__StrToUInt32(&info[1],infoSize-1,&dci->AlignSize)==false)
					return false;
			}
		}
		dci->Align = DataConvertInfo::ALIGN_RIGHT;
		return true;
	}
	if (info[0]=='C')
	{
		if ((infoSize==3) && (info[1]=='%') && (info[2]=='%'))
		{
			dci->ExternalValue[dci->ExternalValuesCount++] = DataConvertInfo::EXTVALUE_AlignSize;
			if (dci->ExternalValuesCount == MAX_EXTVALUE)
				return false;
		} else {
			if (info[1]!=0)
			{
				if (__StrToUInt32(&info[1],infoSize-1,&dci->AlignSize)==false)
					return false;
			}
		}
		dci->Align = DataConvertInfo::ALIGN_CENTER;
		return true;
	}
	// flag de trunchiere
	if (__StrEq(info,infoSize,"trunc",true))
	{
		dci->Flags |= DataConvertInfo::FLAG_TRUNCATE;
		return true;
	}
	if (info[0]=='Z')
	{
		if ((infoSize==3) && (info[1]=='%') && (info[2]=='%'))
		{
			dci->ExternalValue[dci->ExternalValuesCount++] = DataConvertInfo::EXTVALUE_Zecimals;
			if (dci->ExternalValuesCount == MAX_EXTVALUE)
				return false;
		} else {
			if (__StrToUInt32(&info[1],infoSize-1,&dci->Zecimals)==false)
				return false;
		}
		return true;
	}
	if (info[0]=='F')
	{
		if ((infoSize==3) && (info[1]=='%') && (info[2]=='%'))
		{
			dci->ExternalValue[dci->ExternalValuesCount++] = DataConvertInfo::EXTVALUE_FillChar;
			if (dci->ExternalValuesCount == MAX_EXTVALUE)
				return false;
		} else {
			if (infoSize!=2)
				return false;
			dci->FillChar = info[1];
		}
		return true;
	}
	if (info[0]=='G')
	{
		if ((infoSize==3) && (info[1]=='%') && (info[2]=='%'))
		{
			dci->ExternalValue[dci->ExternalValuesCount++] = DataConvertInfo::EXTVALUE_GROUP;
			if (dci->ExternalValuesCount == MAX_EXTVALUE)
				return false;
		} else {
			if (__StrToUInt32(&info[1],infoSize-1,&dci->Group)==false)
				return false;
			if (dci->Group==0)
				return false;
		}
		return true;
	}
	return false;
}
TCHAR* ConvertIntegerNumberToString(DataConvertInfo *dci,TYPE_INT64 data,TCHAR *tempBuffer,int tempBufferSize,int *resultSize)
{
	bool			negativ = false;
	TCHAR*			letters = convert_array_lower;
	int				poz = tempBufferSize-1;
	unsigned int	gPoz=0;
	
	tempBuffer[poz]=0;
	poz--;

	if (dci->Flags & DataConvertInfo::FLAG_UPPER)
		letters = convert_array_upper;

	if (data<0)
	{
		data = -data;
		negativ = true;
	}
	do
	{
		tempBuffer[poz--] = letters[data % dci->Base];
		data = data / dci->Base;
		gPoz++;
		if ((gPoz==dci->Group) && (data>0))
		{
			tempBuffer[poz--]=',';
			gPoz=0;
		}		
	} while ((data>0) && (poz>=2));
	if (data!=0)
		return NULL;
	if (negativ)
		tempBuffer[poz--] = '-';
	// totul e ok
	poz++;
	(*resultSize) = (tempBufferSize-1)-poz;
	return &tempBuffer[poz];
}
TCHAR* ConvertUIntegerNumberToString(DataConvertInfo *dci,TYPE_UINT64 data,TCHAR *tempBuffer,int tempBufferSize,int *resultSize)
{
	TCHAR*			letters = convert_array_lower;
	int				poz = tempBufferSize-1;
	unsigned int	gPoz=0;
	
	tempBuffer[poz]=0;
	poz--;

	if (dci->Flags & DataConvertInfo::FLAG_UPPER)
		letters = convert_array_upper;

	do
	{
		tempBuffer[poz--] = letters[data % dci->Base];
		data = data / dci->Base;
		gPoz++;
		if ((gPoz==dci->Group) && (data>0))
		{
			tempBuffer[poz--]=',';
			gPoz=0;
		}		
	} while ((data>0) && (poz>=2));
	if (data!=0)
		return NULL;
	// totul e ok
	poz++;
	(*resultSize) = (tempBufferSize-1)-poz;
	return &tempBuffer[poz];
}
TCHAR* ConvertDoubleNumberToString(DataConvertInfo *dci,double data,TCHAR *tempBuffer,int tempBufferSize,int *resultSize)
{
	int		pctPoz,size,countZecimals;
	TCHAR*	ptr;

	size = sprintf_s(tempBuffer,tempBufferSize-1,"%lf",data);
	if ((size<0) || (size>=tempBufferSize))
		return NULL;	
	// caut punctul
	ptr = tempBuffer;
	for (pctPoz = 0;((*ptr)!=0) && ((*ptr)!='.');ptr++,pctPoz++);
	if ((*ptr)==0)
	{
		(*ptr)='.';
		size++;
	}
	if (dci->Zecimals==0)
	{
		(*resultSize)=pctPoz;
		return tempBuffer;
	}
	countZecimals = (size-1)-pctPoz;
	if (countZecimals>=(int)dci->Zecimals)
	{
		(*resultSize)=pctPoz+dci->Zecimals+1;
		return tempBuffer;
	}
	ptr = &tempBuffer[size];
	while ((size<tempBufferSize) && (countZecimals<dci->Zecimals))
	{
		(*ptr)='0';
		ptr++;
		size++;
		countZecimals++;
	}
	(*resultSize) = size;
	return tempBuffer;
}
//-------------------------------------------------- FUNCTII STATICE ---------------------------------------------------------------------
int	 GML::Utils::GString::Len(TCHAR *string)
{
	int sz;
	if (string==NULL)
		return 0;
	for (sz=0;string[sz]!=0;sz++);
	return sz;
}
bool GML::Utils::GString::Add(TCHAR *destination, TCHAR *text, int maxDestinationSize, int destinationTextSize, int textSize)
{
	int tr;

	if ((destination==NULL) || (text==NULL))
		return false;
	if (destinationTextSize==-1)
		destinationTextSize = Len(destination);
	if (textSize==-1)
		textSize=Len(text);
	if (destinationTextSize+textSize+1>=maxDestinationSize)
		return false;
	for (tr=0;tr<textSize;tr++,destinationTextSize++)
		destination[destinationTextSize] = text[tr];
	destination[destinationTextSize]=0;

	return true;
}
bool GML::Utils::GString::Set(TCHAR *destination, TCHAR *text, int maxDestinationSize, int textSize)
{
	if (destination==NULL)
		return false;
	destination[0]=0;
	return Add(destination,text,maxDestinationSize,0,textSize);
}
int  GML::Utils::GString::Compare(TCHAR *sir1, TCHAR *sir2, bool ignoreCase)
{
	TCHAR	c1,c2;
	if ((sir1==NULL) || (sir2==NULL))
		return -1;
	for (;((*sir1)!=0) && ((*sir2)!=0);sir1++,sir2++)
	{
		c1=CONVERT_TO_COMPARE((*sir1),ignoreCase);
		c2=CONVERT_TO_COMPARE((*sir2),ignoreCase);
		if (c1<c2)
			return -1;
		if (c1>c2)
			return 1;
	}
	if (((*sir1)==0) && ((*sir2)==0))
		return 0;
	if (((*sir1)==0) && ((*sir2)!=0))
		return -1;
	return 1;
}
bool GML::Utils::GString::Equals(TCHAR *sir1,TCHAR *sir2,bool ignoreCase)
{
	return (bool)(Compare(sir1,sir2,ignoreCase)==0);
}
bool GML::Utils::GString::StartsWith(TCHAR *sir1, TCHAR *sir2, bool ignoreCase)
{
	TCHAR	c1,c2;
	if ((sir1==NULL) || (sir2==NULL))
		return false;
	for (;((*sir1)!=0) && ((*sir2)!=0);sir1++,sir2++)
	{
		c1=CONVERT_TO_COMPARE((*sir1),ignoreCase);
		c2=CONVERT_TO_COMPARE((*sir2),ignoreCase);
		if (c1!=c2)
			return false;
	}
	if ((*sir2)==0)
		return true;
	return false;
}
bool GML::Utils::GString::EndsWith(TCHAR *sir, TCHAR *text, bool ignoreCase, int sirTextSize, int textSize)
{
	if (sirTextSize==-1)
		sirTextSize = GML::Utils::GString::Len(sir);
	if (textSize==-1)
		textSize = GML::Utils::GString::Len(text);
	if (textSize>sirTextSize)
		return false;
	return (bool)(Compare(&sir[sirTextSize-textSize],text,ignoreCase)==0);
}
int  GML::Utils::GString::Find(TCHAR *source, TCHAR *text, bool ignoreCase, int startPoz, bool searchForward, int sourceSize)
{
	if ((source==NULL) || (text==NULL))
		return -1;
	if (sourceSize==-1)
		sourceSize = Len(source);
	if (startPoz<0)
	{
		startPoz = sourceSize+startPoz;
		if (startPoz<0) startPoz=0;
	}
	if (searchForward)
	{
		while (startPoz<sourceSize)
		{
			if (StartsWith(&source[startPoz],text,ignoreCase))
				return startPoz;
			startPoz++;
		}
	} else {
		while (startPoz>=0)
		{
			if (StartsWith(&source[startPoz],text,ignoreCase))
				return startPoz;
			startPoz--;
		}
	}
	return -1;
}
bool GML::Utils::GString::Delete(TCHAR *source,int start,int end,int sourceSize)
{
	if (source==NULL)
		return false;
	if (sourceSize<0)
		sourceSize = Len(source);
	if ((start<0) || (start>end) || (end>sourceSize))
		return false;	
	MEMMOVE(&source[start],&source[end],((sourceSize+1)-end)*sizeof(TCHAR));
	return true;
}
bool GML::Utils::GString::Insert(TCHAR *source,TCHAR *text,int pos,int maxSourceSize,int sourceSize,int textSize)
{
	if ((source==NULL) || (text==NULL) || (maxSourceSize<0) || (pos<0))
		return false;
	if (sourceSize<0)
		sourceSize = Len(source);
	if (textSize<0)
		textSize = Len(text);
	if (pos>sourceSize)
		return false;
	if (sourceSize+textSize+1>=maxSourceSize)
		return false;
	MEMMOVE(&source[pos+textSize],&source[pos],(sourceSize+1-pos)*sizeof(TCHAR));
	MEMCOPY(&source[pos],text,textSize*sizeof(TCHAR));
	return true;
}
bool GML::Utils::GString::ReplaceOnPos(TCHAR *source,TCHAR *text,int start,int end,int maxSourceSize,int sourceSize,int textSize)
{
	int szBuf,newPos;

	if ((source==NULL) || (text==NULL))
		return false;
	if (sourceSize<0)
		sourceSize = Len(source);
	if (textSize<0)
		textSize=Len(text);
	if ((end<start) || (start<0) || (end>sourceSize))
		return false;
	// verific daca incape
	szBuf = end-start;
	newPos = start+textSize;
	if (((sourceSize+textSize+1)-szBuf)>=maxSourceSize)
		return false;
	if (szBuf!=textSize)
	{
		MEMMOVE(&source[newPos],&source[end],(sourceSize+1-end)*sizeof(TCHAR));
	}
	MEMCOPY(&source[start],text,textSize*sizeof(TCHAR));
	return true;
}
bool GML::Utils::GString::Replace(TCHAR *source,TCHAR *pattern,TCHAR *newText,int maxSourceSize,bool ignoreCase,int sourceSize,int patternSize,int newTextSize)
{
	int pos,start;

	if ((source==NULL) || (pattern==NULL) || (newText==NULL))
		return false;
	if (sourceSize<0)
		sourceSize = Len(source);
	if (patternSize<0)
		patternSize = Len(pattern);
	if (newTextSize<0)
		newTextSize = Len(newText);

	start=0;
	do
	{
		pos = Find(source,pattern,ignoreCase,start,true,sourceSize);
		if (pos>=0)
		{
			if (ReplaceOnPos(source,newText,pos,pos+patternSize,maxSourceSize,sourceSize,newTextSize)==false)
				return false;
			start=pos+newTextSize;
		}
	} while (pos>=0);
	return true;
}
bool GML::Utils::GString::ConvertToUInt8(TCHAR *text,unsigned char *value,unsigned int base,int textSize)
{
	TYPE_UINT64	result;
	if (ConvertStringToNumber(text,&result,base,textSize,true)==false)
		return false;
	(*value) = (unsigned char)result;
	return true;
}
bool GML::Utils::GString::ConvertToUInt16(TCHAR *text,unsigned short *value,unsigned int base,int textSize)
{
	TYPE_UINT64	result;
	if (ConvertStringToNumber(text,&result,base,textSize,true)==false)
		return false;
	(*value) = (unsigned short)result;
	return true;
}
bool GML::Utils::GString::ConvertToUInt32(TCHAR *text,unsigned int *value,unsigned int base,int textSize)
{
	TYPE_UINT64	result;
	if (ConvertStringToNumber(text,&result,base,textSize,true)==false)
		return false;
	(*value) = (unsigned int)result;
	return true;
}

bool GML::Utils::GString::ConvertToUInt64(TCHAR *text,TYPE_UINT64 *value,unsigned int base,int textSize)
{
	TYPE_UINT64	result;
	if (ConvertStringToNumber(text,&result,base,textSize,true)==false)
		return false;
	(*value) = result;
	return true;
}

bool GML::Utils::GString::ConvertToInt8(TCHAR *text,char *value,unsigned int base,int textSize)
{
	TYPE_UINT64	result;
	bool		isSigned;
	if (ConvertStringToNumber(text,&result,base,textSize,false,&isSigned)==false)
		return false;
	if (isSigned)
		(*value) = -((char)result);
	else
		(*value) = (char)result;
	return true;
}

bool GML::Utils::GString::ConvertToInt16(TCHAR *text,short *value,unsigned int base,int textSize)
{
	TYPE_UINT64	result;
	bool		isSigned;
	if (ConvertStringToNumber(text,&result,base,textSize,false,&isSigned)==false)
		return false;
	if (isSigned)
		(*value) = -((short)result);
	else
		(*value) = (short)result;
	return true;
}
bool GML::Utils::GString::ConvertToInt32(TCHAR *text,int *value,unsigned int base,int textSize)
{
	TYPE_UINT64	result;
	bool		isSigned;
	if (ConvertStringToNumber(text,&result,base,textSize,false,&isSigned)==false)
		return false;
	if (isSigned)
		(*value) = -((int)result);
	else
		(*value) = (int)result;
	return true;
}

bool GML::Utils::GString::ConvertToInt64(TCHAR *text,TYPE_INT64 *value,unsigned int base,int textSize)
{
	TYPE_UINT64	result;
	bool		isSigned;
	if (ConvertStringToNumber(text,&result,base,textSize,false,&isSigned)==false)
		return false;
	if (isSigned)
		(*value) = -((TYPE_INT64)result);
	else
		(*value) = (TYPE_INT64)result;
	return true;
}
bool GML::Utils::GString::ConvertToDouble(TCHAR *text,double *value,int textSize)
{
	int		punctCount,tr;
	double	val,cCifra,rap;
	bool	negative = false;

	if ((value==NULL) || (text==NULL) || (textSize<1))
		return false;
	val = 0;
	rap = 10;
	tr = 0;
	if (text[0]=='-')
	{
		tr++;
		negative = true;
		text++;
	}
	for (punctCount=0;tr<textSize;tr++,text++)
	{
		if ((*text)=='.')
		{
			punctCount++;
			if (punctCount>1)
				return false;
			continue;
		}
		if (((*text)>='0') && ((*text)<='9'))
		{
			cCifra = (double)((*text)-'0');
			if (punctCount==0) {
				val = val * 10+ cCifra;
			} else {
				val+=cCifra/rap;
				rap*=10;
			}
			continue;
		}
		// nu e un caracter valid
		return false;
	}
	if (negative)
		val = -val;
	(*value) = val;
	return true;
}
bool GML::Utils::GString::ConvertToFloat(TCHAR *text,float *value,int textSize)
{
	double val;
	if (ConvertToDouble(text,&val,textSize)==false)
		return false;
	if (value==NULL)
		return false;
	(*value) = (float)val;
	return true;
}
//--------------------------------------------------- CONSTRUCTORI OBIECT ----------------------------------------------------------------
GML::Utils::GString::GString(void)
{
	Text=NULL;
	Size=MaxSize=0;
}

GML::Utils::GString::~GString(void)
{
	Distroy();
}
void GML::Utils::GString::Distroy()
{
	if (Text!=NULL) delete Text;
	Text=NULL;
	Size=MaxSize=0;
}
bool GML::Utils::GString::Create(int Size)
{
	if (Size<1)
		return false;
	Distroy();
	return Grow(Size);
}
bool GML::Utils::GString::Grow(int newSize)
{
	TCHAR	*temp;
	if (newSize<MaxSize) 
		return true;
	
	temp=Text;
	if ((Text=new TCHAR[newSize])==NULL)
	{
		Text = temp;
		return false;
	}
	if (temp) 
	{ 
		MEMCOPY(Text,temp,Size*sizeof(TCHAR));
		delete temp; 
	} else { 
		Size=0;
		Text[0]=0; 
	}
	MaxSize=newSize;
	return true;
}
//--------------------------------------------------- FUNCTII OBIECT  ---------------------------------------------------------------------
bool GML::Utils::GString::Add(TCHAR *ss,int txSize)
{
	GSTRING_TEST_AND_INIT;	

	if (txSize<0)
		txSize = Len(ss);
	GSTRING_GROW_WITH(txSize);
	if (GML::Utils::GString::Add(Text,ss,MaxSize,Size,txSize))
	{
		Size+=txSize;
		return true;
	}
	return false;
}
bool GML::Utils::GString::AddChar(TCHAR ch)
{
	TCHAR temp[2];
	temp[0]=ch;
	temp[1]=0;
	return Add(temp);
}
bool GML::Utils::GString::AddChars(TCHAR ch,int count)
{
	GSTRING_TEST_AND_INIT;
	return InsertChars(ch,Size,count);
}
bool GML::Utils::GString::Add(GString *ss,int txSize)
{
	GSTRING_TEST_AND_INIT;

	if (ss==NULL)
		return false;
	if (txSize<0)
		txSize = ss->GetSize();
	
	GSTRING_GROW_WITH(txSize);
	if (GML::Utils::GString::Add(Text,ss->GetText(),MaxSize,Size,txSize))
	{
		Size+=txSize;
		return true;
	}
	return false;	
}
bool GML::Utils::GString::Set(TCHAR *ss,int txSize)
{
	GSTRING_TEST_AND_INIT;
	Text[0]=0;
	Size=0;
	return Add(ss,txSize);
}
bool GML::Utils::GString::Set(GString *ss,int txSize)
{
	GSTRING_TEST_AND_INIT;
	Text[0]=0;
	Size=0;
	return Add(ss,txSize);
}


void GML::Utils::GString::Truncate(int newText)
{
	if ((newText<Size) && (newText>=0)) 
	{
		Size=newText;
		Text[Size]=0;
	}
}
int  GML::Utils::GString::Compare(TCHAR *ss,bool ignoreCase)
{
	return Compare(Text,ss,ignoreCase);
}
int  GML::Utils::GString::Compare(GString *ss,bool ignoreCase)
{
	if (ss==NULL)
		return Compare(Text,NULL,ignoreCase);
	else
		return Compare(Text,ss->GetText(),ignoreCase);
}

bool GML::Utils::GString::StartsWith(TCHAR *ss,bool ignoreCase)
{
	return StartsWith(Text,ss,ignoreCase);
}
bool GML::Utils::GString::StartsWith(GString *ss,bool ignoreCase)
{
	if (ss==NULL)
		return false;
	return StartsWith(Text,ss->GetText(),ignoreCase);
}
bool GML::Utils::GString::EndsWith(TCHAR *ss,bool ignoreCase)
{
	return EndsWith(Text,ss,ignoreCase,Size);
}
bool GML::Utils::GString::EndsWith(GString *ss,bool ignoreCase)
{
	if (ss==NULL)
		return false;
	return EndsWith(Text,ss->GetText(),ignoreCase,Size,ss->GetSize());
}
bool GML::Utils::GString::Contains(TCHAR *ss,bool ignoreCase)
{
	return (bool)(Find(ss,ignoreCase)!=-1);
}
bool GML::Utils::GString::Contains(GString *ss,bool ignoreCase)
{
	return (bool)(Find(ss,ignoreCase)!=-1);
}

int	 GML::Utils::GString::Find(TCHAR *ss,bool ignoreCase,int startPoz,bool searchForward)
{
	return Find(Text,ss,ignoreCase,startPoz,searchForward,Size);
}
int	 GML::Utils::GString::Find(GString *ss,bool ignoreCase,int startPoz,bool searchForward)
{
	if (ss==NULL)
		return -1;
	return Find(ss->GetText(),ignoreCase,startPoz,searchForward);
}
int  GML::Utils::GString::FindLast(TCHAR *ss, bool ignoreCase)
{
	if (Size<1)
		return -1;
	return Find(Text,ss,ignoreCase,Size-1,false,-1);
}
int  GML::Utils::GString::FindLast(GString *ss,bool ignoreCase)
{
	if ((Size<1) || (ss==NULL) || (ss->GetText()==NULL) || (ss->GetSize()<1))
		return -1;
	return Find(Text,ss->GetText(),ignoreCase,Size-1,false,ss->GetSize());
}
bool GML::Utils::GString::Equals(TCHAR *ss,bool ignoreCase)
{
	return (Compare(ss,ignoreCase)==0);
}
bool GML::Utils::GString::Equals(GString *ss,bool ignoreCase)
{
	return (Compare(ss,ignoreCase)==0);
}
bool GML::Utils::GString::Delete(int start,int end)
{
	if (Delete(Text,start,end))
	{
		Size-=(end-start);
		return true;
	}
	return false;
}
bool GML::Utils::GString::DeleteChar(int pos)
{
	return Delete(pos,pos+1);
}
bool GML::Utils::GString::Insert(TCHAR *ss,int pos)
{
	int sz = Len(ss);
	
	GSTRING_TEST_AND_INIT;
	GSTRING_GROW_WITH(sz);

	if (Insert(Text,ss,pos,MaxSize,Size,sz)==true)
	{
		Size+=sz;
		return true;
	}
	return false;
}
bool GML::Utils::GString::InsertChar(TCHAR ch,int pos)
{
	TCHAR t[2];
	t[0]=ch;
	t[1]=0;

	return Insert(t,pos);
}
bool GML::Utils::GString::InsertChars(TCHAR ch,int pos,int count)
{
	if (count<1)
		return false;

	GSTRING_TEST_AND_INIT;
	GSTRING_GROW_WITH(count+1);
	MEMMOVE(&Text[pos+count],&Text[pos],(Size-pos)*sizeof(TCHAR));
	for (int tr=0;tr<count;tr++,pos++)
		Text[pos]=ch;	
	Size+=count;
	Text[Size]=0;
	return true;
}
bool GML::Utils::GString::Insert(GString *ss,int pos)
{
	if (ss==NULL)
		return false;

	GSTRING_TEST_AND_INIT;
	GSTRING_GROW_WITH(ss->GetSize());

	if (Insert(Text,ss->GetText(),pos,MaxSize,Size,ss->GetSize())==true)
	{
		Size+=ss->GetSize();
		return true;
	}
	return false;
	
}
bool GML::Utils::GString::ReplaceOnPos(int start,int end,TCHAR *ss,int szSS)
{	
	if ((Text==NULL) || (ss==NULL))
		return false;

	if (szSS<0)
		szSS=Len(ss);

	GSTRING_TEST_AND_INIT;
	GSTRING_GROW_WITH(szSS);

	if (ReplaceOnPos(Text,ss,start,end,MaxSize,Size,szSS)==true)
	{
		Size += szSS;
		Size -= (end-start);
		return true;
	}
	return false;
}
bool GML::Utils::GString::ReplaceOnPos(int start,int end,GString *ss)
{
	if ((Text==NULL) || (ss==NULL))
		return false;

	GSTRING_TEST_AND_INIT;
	GSTRING_GROW_WITH(ss->GetSize());

	return ReplaceOnPos(start,end,ss->GetText(),ss->GetSize());
}

bool GML::Utils::GString::Replace(TCHAR *pattern, TCHAR *newText, bool ignoreCase, int patternSize, int textSize)
{
	int pos;

	if ((Text==NULL) || (pattern==NULL) || (newText==NULL))
		return false;

	GSTRING_TEST_AND_INIT;	
	if (patternSize<0)
		patternSize = Len(pattern);
	if (textSize<0)
		textSize = Len(newText);

	pos = 0;
	do
	{
		pos = Find(Text,pattern,ignoreCase,pos,true,Size);
		if (pos>=0)
		{
			if (ReplaceOnPos(pos,pos+patternSize,newText,textSize)==false)
				return false;
			pos+=textSize;
		}
	} while (pos>=0);
	return true;
}
bool GML::Utils::GString::Replace(GString *pattern,GString *newText,bool ignoreCase)
{
	if ((Text==NULL) || (pattern==NULL) || (newText==NULL))
		return false;
	return Replace(pattern->GetText(),newText->GetText(),ignoreCase,pattern->GetSize(),newText->GetSize());
}
bool GML::Utils::GString::Split(TCHAR *separator, GString *arrayList, int arrayListCount, int *elements,int separatorSize)
{
	int pos,start,count;
	if ((Text==NULL) || (separator==NULL) || (arrayListCount<0))
		return false;
	if ((arrayList!=NULL) && (arrayListCount<1))
		return false;
	if (separatorSize==-1)
		separatorSize = Len(separator);

	start = 0;
	count = 0;
	do
	{
		pos = Find(Text,separator,false,start);
		if (pos>=0)
		{
			if (arrayList!=NULL)
			{
				if (count>=arrayListCount)
					return false;
				if (arrayList[count].Set(&Text[start],pos-start)==false)
					return false;
			}
			count++;
			start = pos+separatorSize;
		}
	} while (pos>=0);
	if ((start>=0) && (start<Size))
	{
		if (arrayList!=NULL)
		{
			if (count>=arrayListCount)
				return false;
			if (arrayList[count].Set(&Text[start],Size-start)==false)
				return false;
		}
		count++;
	}
	if (elements)
		*elements = count;
	return true;
}
bool GML::Utils::GString::Split(GString *separator,GString *arrayList,int arrayListCount,int *elements)
{
	if (separator==NULL)
		return false;
	return Split(separator->GetText(),arrayList,arrayListCount,elements,separator->GetSize());
}
bool GML::Utils::GString::LoadFromFile(TCHAR *fileName,int start,int end)
{
	GML::Utils::File	f;

	if (f.OpenRead(fileName)==false)
		return false;
	while (true)
	{
		if (Grow(f.GetFileSize()+1)==false)
			break;
		if (f.Read(Text,f.GetFileSize())==false)
			break;
		Size = f.GetFileSize();
		Text[Size]=0;
		f.Close();
		return true;
	}
	f.Close();
	return false;
}
bool GML::Utils::GString::Strip(TCHAR *charList,bool stripFromLeft,bool stripFromRight)
{
	int		tr,gr;
	bool	found;

	if (charList==NULL)
		charList = " \t\n\r";
	if (Text==NULL)
		return true;
	if (stripFromLeft)
	{
		for (tr=0;tr<Size;tr++)
		{
			found = false;
			for (gr=0;(charList[gr]!=0) && (found==false);gr++)
				if (Text[tr]==charList[gr])
					found = true;
			if (!found)
				break;
		}
		if (Delete(0,tr)==false)
			return false;
	}

	if (stripFromRight)
	{
		for (tr=Size-1;tr>=0;tr--)
		{
			found = false;
			for (gr=0;(charList[gr]!=0) && (found==false);gr++)
				if (Text[tr]==charList[gr])
					found = true;
			if (!found)
				break;
		}
		tr++;
		if (tr<0)
			tr=0;
		Size = tr;
		Text[Size]=0;
	}
	return true;
}
bool GML::Utils::GString::CopyNextLine(GString *line,int *position)
{
	int start;
	if ((line==NULL) || (position==NULL))
		return false;

	line->Set("");
	start = (*position);
	
	if (((*position)>=Size) || ((*position)<0))
		return false;
	
	while ((*position)<Size)
	{
		if ((Text[(*position)]==13) || (Text[(*position)]==10))
		{
			if (line->Set(&Text[start],(*position)-start)==false)
				return false;
			(*position)++;
			if ((((*position)<Size)) && 
				((Text[(*position)]==13) || (Text[(*position)]==10)) &&
				(Text[(*position)]!=Text[(*position)-1]))
			{
				(*position)++;
			}
			return true;
		}
		(*position)++;
	}
	if (line->Set(&Text[start],(*position)-start)==false)
		return false;
	return true;
}
bool GML::Utils::GString::CopyNext(GString *token,TCHAR *separator,int *position,bool ignoreCase)
{
	int start;

	if ((token==NULL) || (separator==NULL) || (position==NULL))
		return false;
	
	token->Set("");
	start = (*position);
	
	if (((*position)>=Size) || ((*position)<0))
		return false;

	(*position) = Find(separator,ignoreCase,start);

	if ((*position)<0)
	{
		if (token->Set(&Text[start],Size-start)==false)
			return false;
		(*position)=Size;
		return true;
	} else {
		if (token->Set(&Text[start],(*position)-start)==false)
			return false;
		(*position)+=Len(separator);
		return true;
	}
}
bool GML::Utils::GString::CopyPathName(GString *path)
{
	int index = FindLast("\\");
	if (index==-1)
		return path->Set("");
	else
		return path->Set(Text,index);
}
bool GML::Utils::GString::CopyFileName(GString *path)
{
	int index = FindLast("\\");
	if (index==-1)
		return path->Set("");
	else
		return path->Set(&Text[index+1]);
}
bool GML::Utils::GString::PathJoinName(TCHAR *name,TCHAR separator)
{
	TCHAR	txt[2]={separator,0};
	
	if (EndsWith(txt))
	{
		return Add(name);
	} else {
		if (Add(txt)==false)
			return false;
		return Add(name);
	}
}
bool GML::Utils::GString::PathJoinName(GString *name,TCHAR separator)
{
	return PathJoinName(name->GetText(),separator);
}
bool GML::Utils::GString::SetFormated(TCHAR *format, ...)
{
    va_list args;
    int     len;

	GSTRING_TEST_AND_INIT;
    va_start( args, format );
	if ((len = _vscprintf( format, args ))<0)
		return false;
	GSTRING_GROW_WITH(len+1);
    if ((len = vsprintf_s( Text, len+1, format, args ))<0)
		return false;
	Text[len]=0;
	Size = len;
	return true;
}
bool GML::Utils::GString::AddFormated(TCHAR *format, ...)
{
    va_list args;
    int     len;

	GSTRING_TEST_AND_INIT;
    va_start( args, format );
	if ((len = _vscprintf( format, args ))<0)
		return false;
	GSTRING_GROW_WITH(len+1+Size);
    if ((len = vsprintf_s( &Text[Size], len+1, format, args ))<0)
		return false;
	Text[len+Size]=0;
	Size+=(len);
	return true;
}
bool GML::Utils::GString::AddFormatedEx(TCHAR *format, ...)
{
    va_list				args;
    TCHAR*				start;
	DataConvertInfo		dci;
	DataValueUnion		val;
	TCHAR				temp[MAX_FORMATED_EX_TEMP_BUF];
	TCHAR				*toAdd;
	int					toAddSize;
	int					extraAdd;
	unsigned int		tr;


	GSTRING_TEST_AND_INIT;
    va_start( args, format );
	
	start = format;
	while (true)
	{		
		while (((*format)!=0) && ((*format)!='%'))
			format++;
		// adaug ce am gasit pana acuma
		if (Add(start,(int)(format-start))==false)
			return false;
		if ((*format)==0)
			return true;
		if (((*format)=='%') && (format[1]=='{'))
		{
			//procesez datele %{...}
			format+=2;
			ResetDataConvertInfo(&dci);			
			while (((*format)!=0) && ((*format)!='}'))
			{
				start = format;
				while (((*format)!=0) && ((*format)!=',') && ((*format)!='}'))
					format++;
				if (UpdateDataConvertInfo(&dci,start,(int)(format-start))==false)
					return false;
				if ((*format)==',')
					format++;
			}
			// format invalid
			if ((*format)==0)
				return false;
			// sunt pe o acolada
			format++;
			start = format;
			// citesc valoarea
			switch (dci.DataType)
			{
				case DataConvertInfo::TYPE_Int8:
					val.vInt8 = va_arg(args,char);
					break;
				case DataConvertInfo::TYPE_Int16:
					val.vInt16 = va_arg(args,short);
					break;
				case DataConvertInfo::TYPE_Int32:
					val.vInt32 = va_arg(args,int);
					break;
				case DataConvertInfo::TYPE_Int64:
					val.vInt64 = va_arg(args,TYPE_INT64);
					break;

				case DataConvertInfo::TYPE_UInt8:
					val.vUInt8 = va_arg(args,unsigned char);
					break;
				case DataConvertInfo::TYPE_UInt16:
					val.vUInt16 = va_arg(args,unsigned short);
					break;
				case DataConvertInfo::TYPE_UInt32:
					val.vUInt32 = va_arg(args,unsigned int);
					break;
				case DataConvertInfo::TYPE_UInt64:
					val.vUInt64 = va_arg(args,TYPE_UINT64);
					break;

				case DataConvertInfo::TYPE_Char:
					val.vChar = va_arg(args,char);
					break;
				case DataConvertInfo::TYPE_WChar:
					val.vWChar = va_arg(args,short);
					break;

				case DataConvertInfo::TYPE_Bool:
					val.vBool = va_arg(args,bool);
					break;

				case DataConvertInfo::TYPE_Ascii:
					val.vAscii = va_arg(args,char*);
					break;
				case DataConvertInfo::TYPE_Unicode:
					//val.vUnicode = va_arg(args,short*);
					return false;
					break;

				case DataConvertInfo::TYPE_Float:
					val.vFloat= va_arg(args,float);
					break;
				case DataConvertInfo::TYPE_Double:
					val.vDouble= va_arg(args,double);
					break;

				default:
					return false;
			};
			// citesc si valorile externe
			for (tr=0;tr<dci.ExternalValuesCount;tr++)
			{
				switch (dci.ExternalValue[tr])
				{
					case DataConvertInfo::EXTVALUE_Base:
						dci.Base = va_arg(args,unsigned int);
						if ((dci.Base<2) ||  (dci.Base>36))
							return false;
						break;
					case DataConvertInfo::EXTVALUE_AlignSize:
						dci.AlignSize = va_arg(args,unsigned int);
						break;
					case DataConvertInfo::EXTVALUE_FillChar:
						dci.FillChar = va_arg(args,char);
						if (dci.FillChar == 0)
							return false;
						break;
					case DataConvertInfo::EXTVALUE_GROUP:
						dci.Group = va_arg(args,unsigned int);
						if (dci.Group==0)
							return false;
						break;
					case DataConvertInfo::EXTVALUE_Zecimals:
						dci.Zecimals = va_arg(args,unsigned int);
						break;
					default:
						return false;
				};
			}
			// adaug conversia
			switch (dci.DataType)
			{
				case DataConvertInfo::TYPE_Int8:
					toAdd = ConvertIntegerNumberToString(&dci,(TYPE_INT64)val.vInt8,temp,MAX_FORMATED_EX_TEMP_BUF,&toAddSize);
					break;
				case DataConvertInfo::TYPE_Int16:
					toAdd = ConvertIntegerNumberToString(&dci,(TYPE_INT64)val.vInt16,temp,MAX_FORMATED_EX_TEMP_BUF,&toAddSize);
					break;
				case DataConvertInfo::TYPE_Int32:
					toAdd = ConvertIntegerNumberToString(&dci,(TYPE_INT64)val.vInt32,temp,MAX_FORMATED_EX_TEMP_BUF,&toAddSize);
					break;
				case DataConvertInfo::TYPE_Int64:
					toAdd = ConvertIntegerNumberToString(&dci,(TYPE_INT64)val.vInt64,temp,MAX_FORMATED_EX_TEMP_BUF,&toAddSize);
					break;

				case DataConvertInfo::TYPE_UInt8:
					toAdd = ConvertUIntegerNumberToString(&dci,(TYPE_UINT64)val.vUInt8,temp,MAX_FORMATED_EX_TEMP_BUF,&toAddSize);
					break;
				case DataConvertInfo::TYPE_UInt16:
					toAdd = ConvertUIntegerNumberToString(&dci,(TYPE_UINT64)val.vUInt16,temp,MAX_FORMATED_EX_TEMP_BUF,&toAddSize);
					break;
				case DataConvertInfo::TYPE_UInt32:
					toAdd = ConvertUIntegerNumberToString(&dci,(TYPE_UINT64)val.vUInt32,temp,MAX_FORMATED_EX_TEMP_BUF,&toAddSize);
					break;
				case DataConvertInfo::TYPE_UInt64:
					toAdd = ConvertUIntegerNumberToString(&dci,(TYPE_UINT64)val.vUInt64,temp,MAX_FORMATED_EX_TEMP_BUF,&toAddSize);
					break;
				case DataConvertInfo::TYPE_Char:
					toAdd = temp;
					temp[0] = val.vChar;
					toAddSize = 1;
					break;
				case DataConvertInfo::TYPE_WChar:
					toAdd = temp;
					temp[0] = (TCHAR)val.vWChar;
					toAddSize = 1;
					break;
				case DataConvertInfo::TYPE_Bool:
					if (val.vBool)
					{
						toAdd = "True";
						toAddSize = 4;
					} else {
						toAdd = "False";
						toAddSize = 5;
					}
					break;
				case DataConvertInfo::TYPE_Ascii:
					toAdd = val.vAscii;
					toAddSize = Len(toAdd);
					break;
				case DataConvertInfo::TYPE_Float:
					toAdd = ConvertDoubleNumberToString(&dci,(double)val.vFloat,temp,MAX_FORMATED_EX_TEMP_BUF,&toAddSize);
					break;
				case DataConvertInfo::TYPE_Double:
					toAdd = ConvertDoubleNumberToString(&dci,(double)val.vDouble,temp,MAX_FORMATED_EX_TEMP_BUF,&toAddSize);
					break;
				default:
					return false;
			};
			// citire align si align size
			if (dci.AlignSize==0) // fara aliniere
			{
				extraAdd = 0;
			} else {
				extraAdd = dci.AlignSize - toAddSize;
				if ((extraAdd<=0) && (dci.Flags & DataConvertInfo::FLAG_TRUNCATE))
				{
					extraAdd = 0;
					toAddSize = dci.AlignSize;
				}
			}
			switch (dci.Align)
			{
				case DataConvertInfo::ALIGN_LEFT:
					if (Add(toAdd,toAddSize)==false)
						return false;
					if (extraAdd>0)
					{
						if (AddChars(dci.FillChar,extraAdd)==false)
							return false;
					}
					break;
				case DataConvertInfo::ALIGN_RIGHT:
					if (extraAdd>0)
					{
						if (AddChars(dci.FillChar,extraAdd)==false)
							return false;
					}
					if (Add(toAdd,toAddSize)==false)
						return false;
					break;
				case DataConvertInfo::ALIGN_CENTER:
					if ((extraAdd/2)>0)
					{
						if (AddChars(dci.FillChar,extraAdd/2)==false)
							return false;
					}
					if (Add(toAdd,toAddSize)==false)
						return false;
					if ((extraAdd - extraAdd/2)>0)
					{
						if (AddChars(dci.FillChar,extraAdd - extraAdd/2)==false)
							return false;
					}
					break;
			};
		} else {
			// trec la urmatorul element
			start = format;
			format++;
		}
	}
	return true;
}
bool GML::Utils::GString::MatchSimple(TCHAR *mask,bool ignoreCase)
{
	if ((Text==NULL) || (mask==NULL))
		return false;
	return Simple_Match(Text,mask,ignoreCase,false);
}
bool GML::Utils::GString::MatchSimple(GString *mask,bool ignoreCase)
{
	if (mask==NULL)
		return false;
	return MatchSimple(mask->GetText(),ignoreCase);
}
bool GML::Utils::GString::ConvertToInt8(char *value,unsigned int base)
{
	return ConvertToInt8(Text,value,base,Size);
}
bool GML::Utils::GString::ConvertToInt16(short *value,unsigned int base)
{
	return ConvertToInt16(Text,value,base,Size);
}
bool GML::Utils::GString::ConvertToInt32(int *value,unsigned int base)
{
	return ConvertToInt32(Text,value,base,Size);
}
bool GML::Utils::GString::ConvertToInt64(TYPE_INT64 *value,unsigned int base)
{
	return ConvertToInt64(Text,value,base,Size);
}
bool GML::Utils::GString::ConvertToUInt8(unsigned char *value,unsigned int base)
{
	return ConvertToUInt8(Text,value,base,Size);
}
bool GML::Utils::GString::ConvertToUInt16(unsigned short *value,unsigned int base)
{
	return ConvertToUInt16(Text,value,base,Size);
}
bool GML::Utils::GString::ConvertToUInt32(unsigned int *value,unsigned int base)
{
	return ConvertToUInt32(Text,value,base,Size);
}
bool GML::Utils::GString::ConvertToUInt64(TYPE_UINT64 *value,unsigned int base)
{
	return ConvertToUInt64(Text,value,base,Size);
}
bool GML::Utils::GString::ConvertToDouble(double *value)
{
	return ConvertToDouble(Text,value,Size);
}
bool GML::Utils::GString::ConvertToFloat(float *value)
{
	return ConvertToFloat(Text,value,Size);
}
//------------------------------- OLD -----------------------------
void GML::Utils::GString::NumberToString(TYPE_UINT64 number, unsigned int base, int max_size , int fill)
{
	unsigned char	rest;
	int				initialSize;

	if ((max_size<0) || (max_size>=MaxSize)) max_size=MaxSize-1;
	Size=initialSize=max_size;
	Text[max_size--]=0;
	do
	{
		rest=(unsigned char)(number%base);
		Text[max_size--]=convert_array_upper[rest];
		number/=base;
	} while ((max_size>=0) && (number>0));
	// nu s-a convertit complet
	if (number>0) { Text[0]=Text[1]='.'; return; }
	// s-a convertiti complet
	if (fill>0) { memset(Text,fill,max_size+1); return; }
	// mut datele
	memmove(Text,&Text[max_size+1],initialSize-max_size);
	Size=initialSize-max_size;
	Size--;
}
