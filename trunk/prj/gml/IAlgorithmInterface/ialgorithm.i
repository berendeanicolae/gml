%module ialgorithm
%include "std_string.i"

%{
#define SWIG_FILE_WITH_INIT
#include "..\..\..\code\GmlLib\inc\Compat.h";
#include "..\..\..\code\GmlLib\inc\IAlgorithm.h";
%}

#include "..\..\..\code\GmlLib\inc\Compat.h";

namespace GML
{
	namespace Algorithm
	{
		class IAlgorithm
		{
		public:

			%feature("autodoc", "1");
			IAlgorithm();

			%feature("autodoc", "1");
			bool			SetProperty(GML::Utils::AttributeList &config);

			%feature("autodoc", "1");
			bool			SetProperty(char *config);

			%feature("autodoc", "1");
			bool			GetProperty(GML::Utils::AttributeList &config);
									
			%feature("autodoc", "1");
			virtual bool	Init() = 0;

			%feature("autodoc", "1");
			void	OnExecute(char *command);
			
			%feature("autodoc", "1");			
			bool	Execute(char *command);
			
			%feature("autodoc", "1");
			bool	Wait();

			%feature("autodoc", "1");
			bool	Wait(unsigned int nrMiliseconds);
		};
	}
}







