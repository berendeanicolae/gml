%module ialgorithm
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
			IAlgorithm();
			%feature("autodoc", "1");
			virtual bool	SetProperty(GML::Utils::AttributeList &config) = 0;
			%feature("autodoc", "1");
			virtual bool	GetConfiguration(GML::Utils::AttributeList &config) = 0;
			%feature("autodoc", "1");
			virtual bool	Init() = 0;
			%feature("autodoc", "1");
			virtual void	Execute(unsigned int command)=0;
			%feature("autodoc", "1");
			void	OnExecute(char *command);
		};
	}
}







