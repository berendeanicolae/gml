#pragma once

#include "Compat.h"
#include "GString.h"
#include "MD5.h"

namespace GML
{
	namespace DB
	{
		namespace TYPES
		{
			enum 
			{
				BOOLEAN,
				UINT8,
				UINT16,
				UINT32,
				UINT64,
				INT8,
				INT16,
				INT32,
				INT64,
				FLOAT,
				DOUBLE,
				ASCII,
				UNICODE,
				HASH,

				UNKNOWN
			};
		}
		namespace COLUMNTYPE
		{
			enum {
				UNKNOWN,
				LABEL,
				HASH,
				FEATURE,
			};
		};
		struct EXPORT RecordHash
		{
		public:
			union
			{
				UInt8		bValue[16];
				UInt32		dwValue[4];
			} Hash;
		public:
			bool	CreateFromText(char *text);
			bool	ToString(GML::Utils::GString &str);
			void	Copy(RecordHash &rHash);
			void	Reset();
			bool	ComputeHashForBuffer(void *buffer,unsigned int bufferSize);
			bool	ComputeHashForText(char *text);
		};
		struct EXPORT ColumnInfo
		{
			UInt32			DataType;
			UInt32			ColumnType;
			UInt32			NameIndex;
			char*			Name;			
		};
		struct EXPORT DBRecord 
		{
			UInt32			Type;
			union
			{
				bool		BoolVal;
				Int8		Int8Val;
				Int16		Int16Val;
				Int32		Int32Val;
				Int64		Int64Val;
				UInt8		UInt8Val;
				UInt16		UInt16Val;
				UInt32		UInt32Val;
				UInt64		UInt64Val;
				float		FloatVal;
				double		DoubleVal;
				char*		AsciiStrVal;
				wchar_t*	UnicodeStrVal;
				RecordHash	Hash;
			} Value;
		};
	}
}

