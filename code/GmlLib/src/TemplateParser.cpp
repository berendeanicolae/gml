#include "TemplateParser.h"

#define TCHTYPE_AD			0
#define TCHTYPE_AI			1
#define TCHTYPE_PRD			2
#define TCHTYPE_PRI			3
#define TCHTYPE_PPD			4
#define TCHTYPE_PPI			5
#define TCHTYPE_CRLF		6
#define TCHTYPE_STRING		7
#define TCHTYPE_COMMENT		8
#define TCHTYPE_SEP			9
#define TCHTYPE_TERM		10
#define TCHTYPE_EQ			11
#define TCHTYPE_WORD		12
#define TCHTYPE_ASCII		13
#define TCHTYPE_UNK			14

int TemplateFileGetCharType(unsigned char ch)
{
	if (ch=='{')
		return TCHTYPE_AD;
	if (ch=='}')
		return TCHTYPE_AI;
	if (ch=='[')
		return TCHTYPE_PPD;
	if (ch==']')
		return TCHTYPE_PPI;
	if (ch=='(')
		return TCHTYPE_PRD;
	if (ch==')')
		return TCHTYPE_PRI;
	if ((ch=='\n') || (ch=='\r'))
		return TCHTYPE_CRLF;
	if ((ch=='"') || (ch=='\''))
		return TCHTYPE_STRING;
	if (ch=='#')
		return TCHTYPE_COMMENT;
	if ((ch==',') || (ch==' ') || (ch=='\t'))
		return TCHTYPE_SEP;
	if (ch==';')
		return TCHTYPE_TERM;
	if (ch=='=')
		return TCHTYPE_EQ;
	if ((ch>='A') && (ch<='Z'))
		return TCHTYPE_WORD;
	if ((ch>='a') && (ch<='z'))
		return TCHTYPE_WORD;
	if ((ch>='0') && (ch<='9'))
		return TCHTYPE_WORD;
	if ((ch=='.') || (ch=='_') || (ch=='-') || (ch=='+') || (ch=='%'))
		return TCHTYPE_WORD;
	if ((ch>=' ') && (ch<=127))
		return TCHTYPE_ASCII;
	return TCHTYPE_UNK;
}
//==============================================================================================
int FindEOL(unsigned char *ptr,unsigned int start,unsigned int size)
{
	ptr+=start;
	while ((start<size) && (TemplateFileGetCharType(*ptr)!=TCHTYPE_CRLF))
	{
		start++;
		ptr++;
	}
	return start;
}
int SkipSimilar(unsigned char *ptr,unsigned int start,unsigned int size)
{
	int tip;

	ptr+=start;
	tip = TemplateFileGetCharType(*ptr);
	while ((start<size) && (TemplateFileGetCharType(*ptr)==tip))
	{
		start++;
		ptr++;
	}
	return start;
}
int FindTypeCast(unsigned char *ptr,unsigned int start,unsigned int size)
{
	// se intelege ca incep cu un (
	start++;
	ptr+=start;		
	while ((start<size) && ((*ptr)!=')') && ((*ptr)>=32))
	{
		start++;
		ptr++;
	}
	if (start>=size)
		return -1;
	if ((*ptr)==')')
		return start+1;
	return -1;
}
int FindString(unsigned char *ptr,unsigned int start,unsigned int size)
{
	ptr+=start;
	unsigned char s_term = (*ptr);
	ptr++;
	start++;
	while ((start<size) && ((*ptr)!=s_term))
	{
		start++;
		ptr++;
	}
	if (start>=size)
		return -1;
	if ((*ptr)==s_term)
		return start+1;
	return -1;
}
int FindBlock(unsigned char *ptr,unsigned int start,unsigned int size)
{
	int acolade = 0;
	int ppatrate = 0;
	int tip;
	int next;

	do
	{
		tip = TemplateFileGetCharType(ptr[start]);
		switch (tip)
		{
			case TCHTYPE_SEP:
			case TCHTYPE_CRLF:
			case TCHTYPE_WORD:
			case TCHTYPE_ASCII:
				next = SkipSimilar(ptr,start,size);
				break;
			case TCHTYPE_COMMENT:
				next = FindEOL(ptr,start,size);
				break;
			case TCHTYPE_STRING:
				next = FindString(ptr,start,size);
				break;
			case TCHTYPE_PRD:
				next = FindTypeCast(ptr,start,size);
				break;
			case TCHTYPE_AD:
				acolade++;
				next = start+1;
				break;
			case TCHTYPE_AI:
				acolade--;
				next = start+1;
				break;
			case TCHTYPE_PPD:
				ppatrate++;
				next = start+1;
				break;
			case TCHTYPE_PPI:
				ppatrate--;
				next = start+1;
				break;
			case TCHTYPE_EQ:
			case TCHTYPE_TERM:
				next = start+1;
				break;
			case TCHTYPE_UNK:
				return -1;
				break;
			default:
				return -1;
				break;
		};
		start = next;
		if ((acolade<0) || (ppatrate<0))
			return -1;
		if ((acolade==0) && (ppatrate==0))
			break;
		
	} while (start<size);
	if ((acolade==0) && (ppatrate==0))
		return start;
	return -1;	
}
GML::Utils::TemplateParser::TemplateParser()
{
	Tokens.Create(1024);
	Error.Set("");
}
bool GML::Utils::TemplateParser::AddToken(unsigned int start,unsigned int end,unsigned int Type)
{
	Token	t;
	t.Start = start;
	t.End = end;
	t.Type = Type;
	if (Tokens.PushByRef(t)==false)
	{
		Error.SetFormated("[GML:TemplateParser] -> Unable to allocate memory for token");
		return false;
	}
	return true;
}
bool GML::Utils::TemplateParser::AddTerminator(unsigned int start)
{
	// nu se adauga la inceput sau dupa un alt terminator
	if (Tokens.Len()==0)
		return true;
	if (Tokens[Tokens.Len()-1].Type==TOKEN_TERM)
		return true;
	return AddToken(start,start,TOKEN_TERM);
}
bool GML::Utils::TemplateParser::Parse(char *_Text,int TextSize)
{
	int				start,next,cType;
	unsigned char	*ptr = (unsigned char *)_Text;
	bool			startLine = true;
	
	Error.Set("");
	if (Text.Set(_Text,TextSize)==false)
	{
		Error.Set("[GML:TemplateParser] -> Unable to alloc memory for parse buffer");
		return false;
	}
	if (Tokens.DeleteAll()==false)
	{
		Error.Set("[GML:TemplateParser] -> Failed to delete all tokens");
		return false;
	}	
	start = 0;
	while (start<TextSize)
	{		
		cType = TemplateFileGetCharType(ptr[start]);
		switch (cType)
		{
			case TCHTYPE_SEP:
				next = SkipSimilar(ptr,start,TextSize);
				break;
			case TCHTYPE_COMMENT:
				next = FindEOL(ptr,start,TextSize);
				break;
			case TCHTYPE_STRING:
				next = FindString(ptr,start,TextSize);
				if (next==-1)
				{
					Error.SetFormated("[GML:TemplateParser] -> Invalid string => %s",&ptr[start]);
					return false;
				}
				if (AddToken(start,next,TOKEN_STRING)==false)
					return false;
				break;
			case TCHTYPE_CRLF:
				next = SkipSimilar(ptr,start,TextSize);
				if (AddTerminator(start)==false)
					return false;
				break;
			case TCHTYPE_PRD:
				next = FindTypeCast(ptr,start,TextSize);
				if (next==-1)
				{
					Error.SetFormated("[GML:TemplateParser] -> Invalid cast method => %s",&ptr[start]);
					return false;
				}
				if (AddToken(start,next,TOKEN_CAST)==false)
					return false;
				break;
			case TCHTYPE_ASCII:
				next = SkipSimilar(ptr,start,TextSize);
				if (AddToken(start,next,TOKEN_ASCII)==false)
					return false;
				break;
			case TCHTYPE_WORD:
				next = SkipSimilar(ptr,start,TextSize);
				if (AddToken(start,next,TOKEN_WORD)==false)
					return false;
				break;
			case TCHTYPE_TERM:
				next = start+1;
				if (AddTerminator(start)==false)
					return false;
				break;
			case TCHTYPE_EQ:
				next = start+1;
				if (AddToken(start,next,TOKEN_EQ)==false)
					return false;
				break;
			case TCHTYPE_AD:
				next = FindBlock(ptr,start,TextSize);
				if (next==-1)
				{
					Error.SetFormated("[GML:TemplateParser] -> Invalid block ( invalid number of } or ] ) => %s",&ptr[start]);
					return false;
				}
				if (AddToken(start,next,TOKEN_DICT)==false)
					return false;
				break;
			case TCHTYPE_PPD:
				next = FindBlock(ptr,start,TextSize);
				if (next==-1)
				{
					Error.SetFormated("[GML:TemplateParser] -> Invalid block ( invalid number of } or ] ) => %s",&ptr[start]);
					return false;
				}
				if (AddToken(start,next,TOKEN_LIST)==false)
					return false;
				break;
			case TCHTYPE_UNK:
				Error.SetFormated("[GML:TemplateParser] -> Unknwon charater (0x%02X) -> %c",ptr[start],ptr[start]);
				return false;
			default:
				Error.SetFormated("[GML:TemplateParser] -> Unknwon charater (0x%02X) -> %c",ptr[start],ptr[start]);
				return false;
		}
		if (next<=start)
		{
			Error.SetFormated("[GML:TemplateParser] -> Invalid token => %s",&ptr[start]);
			return false;
		}
		start = next;
	}
	return AddTerminator(start);
}
unsigned int GML::Utils::TemplateParser::GetCount()
{
	return Tokens.Len();
}
bool GML::Utils::TemplateParser::Get(unsigned int index,GML::Utils::GString &str,unsigned int &type)
{
	Token *t = Tokens.GetPtrToObject(index);
	if (t==NULL)
		return false;
	type = t->Type;
	if (type == TOKEN_STRING)
		return str.Set(&Text.GetText()[t->Start+1],t->End-t->Start-2);
	return str.Set(&Text.GetText()[t->Start],t->End-t->Start);
}
bool GML::Utils::TemplateParser::Get(unsigned int start,unsigned int end,GML::Utils::GString &str)
{
	Token * t= Tokens.GetPtrToObject(0);
	unsigned int len = Tokens.Len();

	if ((start>end) || (end>=len))
		return false;

	return str.Set(&Text.GetText()[t[start].Start],t[end].End-t[start].Start);
}
bool GML::Utils::TemplateParser::FindNext(unsigned int &start,unsigned int &end)
{
	Token * t= Tokens.GetPtrToObject(0);
	unsigned int len = Tokens.Len();

	if ((t==NULL) || (start>=len))
		return false;
	
	t+=start;
	while ((start<len) && (t->Type==TOKEN_TERM))
	{
		t++;
		start++;
	}
	if (start>=len)
		return false;
	end = start;
	while ((end<len) && (t->Type!=TOKEN_TERM))
	{
		t++;
		end++;
	}
	return true;
}
char*GML::Utils::TemplateParser::GetError()
{
	return Error.GetText();
}