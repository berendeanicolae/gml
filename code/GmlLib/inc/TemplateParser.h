#ifndef __TEMPLATE_PARSER__
#define __TEMPLATE_PARSER__

#include "GString.h"
#include "GTFVector.h"

namespace GML
{
	namespace Utils
	{		
		class EXPORT TemplateParser
		{
		public:
			enum
			{
				TOKEN_CAST = 0,
				TOKEN_STRING,
				TOKEN_TERM,
				TOKEN_WORD,
				TOKEN_ASCII,
				TOKEN_EQ,
				TOKEN_LIST,
				TOKEN_DICT
			};

			struct Token
			{
				unsigned int	Start;
				unsigned int	End;
				unsigned int	Type;
			};
			GML::Utils::GString				Text;
			GML::Utils::GTFVector<Token>	Tokens;

			bool							AddToken(unsigned int start,unsigned int end,unsigned int Type);
			bool							AddTerminator(unsigned int start);
		public:
			TemplateParser();
			bool							Parse(char *_Text,int TextSize = -1);
			unsigned int					GetCount();
			bool							Get(unsigned int index,GML::Utils::GString &str,unsigned int &type);
			bool							Get(unsigned int start,unsigned int end,GML::Utils::GString &str);
			bool							FindNext(unsigned int &start,unsigned int &end);	
		};
	}
};


#endif
