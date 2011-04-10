#pragma once

#include "Compat.h"
#include "GString.h"

namespace GML
{
	namespace DB
	{
		enum DBRecordDataType
		{
			NULLVAL = 0,
			BOOLVAL,
			UINT8VAL,
			UINT16VAL,
			UINT32VAL,
			UINT64VAL,
			INT8VAL,
			INT16VAL,
			INT32VAL,
			RAWPTRVAL,
			BYTESVAL,
			ASCIISTTVAL,
			UNICSTRVAL,
			DOUBLEVAL,
			FLOATVAL,
			HASHVAL
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
		};
		struct EXPORT DBRecord 
		{
			UInt32			Type;
			char*			Name;
			UInt32			Size;
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
				void*		RawPtrVal;
				UInt8*		BytesVal;
				double		DoubleVal;
				float		FloatVal;
				char*		AsciiStrVal;
				wchar_t*	UnicStrVal;
				RecordHash	Hash;
			};
		};
	}
}

