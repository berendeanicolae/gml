#pragma once

#include "compat.h"

namespace GML
{
	namespace Utils
	{ 
		class EXPORT MD5
		{
			struct MD5Context 
			{
				unsigned int	buf[4];
				unsigned int	bits[2];
				unsigned char	in[64]; 
				unsigned char	digest[16];
				char			stringResult[33];
			};

			MD5Context		context;
		public:
			void			Init();
			void			Update(void *Buffer,unsigned int size);
			void			Final();
			char*			GetResult();		
		};
	}
}

