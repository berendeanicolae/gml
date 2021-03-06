#pragma once

#include "compat.h"
#include "GString.h"
#include "GTVector.h"
#include "File.h"
#include "TemplateParser.h"



namespace GML
{
	namespace Utils
	{
		enum AttributeFlags
		{
			FL_LIST = 1,
			FL_FILEPATH = 2,
			FL_FOLDER = 4,
			FL_BITSET = 8,
		};
		struct EXPORT Attribute
		{
			char*			Name;
			char*			MetaData;
			unsigned char*	Data;
			unsigned int	DataSize;
			unsigned int	AttributeType;
			unsigned int	ElementsCount;

			bool operator < (Attribute &a1);
			bool operator > (Attribute &a1);

			bool			GetListItems(GML::Utils::GString &str);
			bool			GetBitSetItems(GML::Utils::GString &str);
			bool			GetDescription(GML::Utils::GString &str);
			UInt32			GetFlags();
		};

		struct EXPORT AttributeLink
		{
			char*			Name;
			void*			LocalAddress;
			unsigned int	AttributeType;
			char*			MetaData;
		};

		class EXPORT AttributeList
		{
			GTVector<GML::Utils::Attribute>	list;
			TemplateParser					tp;
			GML::Utils::GString				error;
			
			bool			OldFromString(GML::Utils::GString &text);
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
		
			bool			AddAttribute(char* Name,void *Data,unsigned int AttributeType,unsigned int ElementsCount=1,char *Description=NULL);

			bool			AddString(char *Name, char *Text, char *Description = NULL);
			bool			AddBool(char *Name, bool value,char *Description = NULL);
			bool			AddDouble(char *Name, double value, char *Description = NULL);
			bool			AddUInt32(char *Name, UInt32 value, char *Description = NULL);
			bool			AddInt32(char *Name, Int32 value, char *Description = NULL);

			bool			Update(char *Name,void *Data,UInt32 DataSize);

			bool			UpdateBool(char *Name,bool &boolValue,bool useDefault=false,bool defaultValue=false);
			bool			UpdateUInt32(char *Name,UInt32 &uint32Value,bool useDefault=false,UInt32 defaultValue=0);
			bool			UpdateInt32(char *Name,Int32 &int32Value,bool useDefault=false,Int32 defaultValue=0);
			bool			UpdateDouble(char *Name,double &doubleValue,bool useDefault=false,double defaultValue=0);
			bool			UpdateString(char *Name,GML::Utils::GString &text,bool useDefault=false,char* defaultValue="");
			
			void			Clear();
			Attribute*		Get(unsigned int index);
			Attribute*		Get(char* Name);
			unsigned int	GetCount();

			bool			Save(char *fileName);
			bool			Load(char *fileName);
			bool			Create(char *text,char separator=';');		
			char*			GetError();	
		};
	}
}