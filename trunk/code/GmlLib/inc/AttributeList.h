#pragma once

#include "compat.h"
#include "GString.h"
#include "GTVector.h"
#include "File.h"

namespace GML
{
	namespace Utils
	{
		struct EXPORT Attribute
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

		class EXPORT AttributeList
		{
			GML::Utils::GTVector<GML::Utils::Attribute>	list;
			
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
			bool			UpdateString(char *Name,GML::Utils::GString &text);

			void			Clear();
			Attribute*		Get(unsigned int index);
			Attribute*		Get(char* Name);
			unsigned int	GetCount();

			bool			Save(char *fileName);
			bool			Load(char *fileName);
			bool			Create(char *text,char separator=';');
		};
	}
}