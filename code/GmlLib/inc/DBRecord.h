#pragma once

#include "Compat.h"

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
			UInt8		Value[16];
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

