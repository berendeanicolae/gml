#pragma once

#include "Windows.h"

#define TCHAR				char
#define TYPE_UINT64			unsigned __int64
#define TYPE_INT64			__int64	

class GString
{
	TCHAR	*Text;
	int		Size;
	int		MaxSize;

public:
	bool	Grow(int newSize);
public:
	GString(void);
	~GString(void);

// functii statice
	static int		Len(TCHAR *string);
	static bool		Add(TCHAR *destination,TCHAR *text,int maxDestinationSize,int destinationTextSize = -1,int textSize = -1);	
	static bool		Set(TCHAR *destination,TCHAR *text,int maxDestinationSize,int textSize = -1);
	static bool		Equals(TCHAR *sir1,TCHAR *sir2,bool ignoreCase=false);
	static int		Compare(TCHAR *sir1,TCHAR *sir2,bool ignoreCase=false);
	static bool		StartsWith(TCHAR *sir,TCHAR *text,bool ignoreCase=false);					
	static bool		EndsWith(TCHAR *sir,TCHAR *text,bool ignoreCase=false,int sirTextSize = -1,int textSize = -1);
	static int		Find(TCHAR *source,TCHAR *text,bool ignoreCase=false,int startPoz=0,bool searchForward=true,int sourceSize = -1);
	static bool		Delete(TCHAR *source,int start,int end,int sourceSize=-1);
	static bool		Insert(TCHAR *source,TCHAR *text,int pos,int maxSourceSize,int sourceSize=-1,int textSize=-1);
	static bool		ReplaceOnPos(TCHAR *source,TCHAR *text,int start,int end,int maxSourceSize,int sourceSize=-1,int textSize=-1);
	static bool		Replace(TCHAR *source,TCHAR *pattern,TCHAR *newText,int maxSourceSize,bool ignoreCase=false,int sourceSize=-1,int patternSize=-1,int newTextSize=-1);
	static bool		ConvertToUInt8(TCHAR *text,unsigned char *value,unsigned int base=0,int textSize=-1);
	static bool		ConvertToUInt16(TCHAR *text,unsigned short *value,unsigned int base=0,int textSize=-1);
	static bool		ConvertToUInt32(TCHAR *text,unsigned int *value,unsigned int base=0,int textSize=-1);
	static bool		ConvertToUInt64(TCHAR *text,TYPE_UINT64 *value,unsigned int base=0,int textSize=-1);
	static bool		ConvertToInt8(TCHAR *text,char *value,unsigned int base=0,int textSize=-1);
	static bool		ConvertToInt16(TCHAR *text,short *value,unsigned int base=0,int textSize=-1);
	static bool		ConvertToInt32(TCHAR *text,int *value,unsigned int base=0,int textSize=-1);
	static bool		ConvertToInt64(TCHAR *text,TYPE_INT64 *value,unsigned int base=0,int textSize=-1);
	static bool		ConvertToDouble(TCHAR *text,double *value,int textSize=-1);
	static bool		ConvertToFloat(TCHAR *text,float *value,int textSize=-1);

// functii obiect
	operator		TCHAR *() { return Text; }

	char			*GetText() { return Text; }
	int				GetSize() { return Size; }
	int				Len() { return Size; }

	bool			Add(TCHAR *ss,int size=-1);
	bool			AddChar(char ch);
	bool			Add(GString *ss,int size=-1);

	bool			Set(TCHAR *ss,int size=-1);
	bool			Set(GString *ss,int size=-1);

	bool			SetFormated(TCHAR *format,...);
	bool			AddFormated(TCHAR *format,...);

	bool			Create(int initialSize=64);
	bool			LoadFromFile(TCHAR *fileName,int start=0,int end=-1);
	void			Distroy();
	void			Truncate(int newSize);

	int				Compare(TCHAR *ss,bool ignoreCase=false);
	int				Compare(GString *ss,bool ignoreCase=false);

	bool			StartsWith(TCHAR *ss,bool ignoreCase=false);
	bool			StartsWith(GString *ss,bool ignoreCase=false);
	bool			EndsWith(TCHAR *ss,bool ignoreCase=false);
	bool			EndsWith(GString *ss,bool ignoreCase=false);
	bool			Contains(TCHAR *ss,bool ignoreCase=false);
	bool			Contains(GString *ss,bool ignoreCase=false);
	bool			Equals(TCHAR *ss,bool ignoreCase=false);
	bool			Equals(GString *ss,bool ignoreCase=false);

	int				Find(TCHAR *ss,bool ignoreCase=false,int startPoz=0,bool searchForward=true);
	int				Find(GString *ss,bool ignoreCase=false,int startPoz=0,bool searchForward=true);
	int				FindLast(TCHAR *ss,bool ignoreCase=false);
	int				FindLast(GString *ss,bool ignoreCase=false);

	bool			Delete(int start,int end);
	bool			DeleteChar(int pos);
	bool			Insert(TCHAR *ss,int pos);
	bool			InsertChar(TCHAR ch,int pos);
	bool			Insert(GString *ss,int pos);
	bool			ReplaceOnPos(int start,int end,TCHAR *newText,int newTextSize=-1);
	bool			ReplaceOnPos(int start,int end,GString *ss);
	bool			Replace(TCHAR *pattern,TCHAR *newText,bool ignoreCase=false,int patternSize = -1,int newTextSize = -1);
	bool			Replace(GString *pattern,GString *newText,bool ignoreCase=false);
	bool			Strip(TCHAR *charList=NULL,bool stripFromLeft=true,bool stripFromRight=true);

	bool			Split(TCHAR *separator,GString *arrayList,int arrayListCount,int *elements=NULL,int separatorSize=-1);
	bool			Split(GString *separator,GString *arrayList,int arrayListCount,int *elements=NULL);

	bool			CopyNextLine(GString *line,int *position);
	bool			CopyNext(GString *token,TCHAR *separator,int *position,bool ignoreCase=false);
		
	bool			CopyPathName(GString *path);
	bool			CopyFileName(GString *path);
	bool			PathJoinName(TCHAR *name,TCHAR separator = '\\');
	bool			PathJoinName(GString *name,TCHAR separator = '\\');

	bool			MatchSimple(TCHAR *mask,bool ignoreCase=false);
	bool			MatchSimple(GString *mask,bool ignoreCase=false);

	bool			ConvertToInt8(char *value,unsigned int base=0);
	bool			ConvertToInt16(short *value,unsigned int base=0);
	bool			ConvertToInt32(int *value,unsigned int base=0);
	bool			ConvertToInt64(TYPE_INT64 *value,unsigned int base=0);

	bool			ConvertToUInt8(unsigned char *value,unsigned int base=0);
	bool			ConvertToUInt16(unsigned short *value,unsigned int base=0);
	bool			ConvertToUInt32(unsigned int *value,unsigned int base=0);
	bool			ConvertToUInt64(TYPE_UINT64 *value,unsigned int base=0);

	bool			ConvertToDouble(double *value);
	bool			ConvertToFloat(float *value);

	void			NumberToString(TYPE_UINT64 number,unsigned int base,int max_size=-1,int fill=-1);


};

