#pragma once

#include "Compat.h"

namespace GML
{
	namespace DB
	{
		enum DBRecordDataType
		{
			NULLVAL = 0,
			UINT8VAL,
			UINT16VAL,
			UINT32VAL,
			UINT64VAL,
			RAWPTRVAL,
			BYTESVAL,
			ASCIISTTVAL,
			UNICSTRVAL,
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
			union
			{
				UInt8		UInt8Val;
				UInt16		UInt16Val;
				UInt32		UInt32Val;
				UInt64		UInt64Val;
				void*		RawPtrVal;
				UInt8*		BytesVal;
				char*		AsciiStrVal;
				wchar_t*	UnicStrVal;
				RecordHash	Hash;
			};
		};
	}
}

