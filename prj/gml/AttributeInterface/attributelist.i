%module attributelist
%include "std_string.i"

%{
#define SWIG_FILE_WITH_INIT
#include "..\..\..\code\GmlLib\inc\AttributeList.h";
#include "..\..\..\code\GmlLib\inc\Compat.h";
%}

namespace GML
{
	namespace Utils
	{
		struct Attribute
		{
			char*			Name;
			char*			Description;
			unsigned char*	Data;
			unsigned int	DataSize;
			unsigned int	AttributeType;
			unsigned int	ElementsCount;

			bool operator < (Attribute &a1);
			bool operator > (Attribute &a1);
		};

		class AttributeList
		{
			GML::Utils::GTVector<GML::Utils::Attribute>	list;
			%feature("autodoc", "1");
			bool			FromString(GML::Utils::GString &text);
		public:
			enum 
			{
				BOOLEAN = 0,
				INT8,INT16,INT32,INT64,
				UINT8,UINT16,UINT32,UINT64,
				FLOAT,DOUBLE,
				STRING,
				ATTRIBUTES_COUNT		
			};		
		public:
			AttributeList(void);
			~AttributeList(void);
		    %feature("autodoc", "1");
			bool			AddAttribute(char* Name,void *Data,unsigned int AttributeType,unsigned int ElementsCount=1,char *Description=NULL);
			%feature("autodoc", "1");
			bool			AddString(char *Name, char *Text, char *Description = NULL);
			%feature("autodoc", "1");
			bool			AddBool(char *Name, bool value,char *Description = NULL);
			%feature("autodoc", "1");
			bool			AddDouble(char *Name, double value, char *Description = NULL);
			//%feature("autodoc", "1");
			//bool			AddUInt32(char *Name, UInt32 value, char *Description = NULL);
			%feature("autodoc", "1");
			bool			AddInt32(char *Name, int value, char *Description);
			%feature("autodoc", "1");
			bool			Update(char *Name,void *Data,UInt32 DataSize);
			%feature("autodoc", "1");
			//bool			UpdateString(char *Name,GML::Utils::GString &text);
			string			UpdateString(char *Name);
			%feature("autodoc", "1");
			void			Clear();
			%feature("autodoc", "1");
			Attribute*		Get(unsigned int index);
			%feature("autodoc", "1");
			Attribute*		Get(char* Name);
			%feature("autodoc", "1");
			unsigned int	GetCount();
			%feature("autodoc", "1");
			bool			Save(char *fileName);
			%feature("autodoc", "1");
			bool			Load(char *fileName);
			%feature("autodoc", "1");
			bool			Create(char *text,char separator=';');
		};
	}
}







