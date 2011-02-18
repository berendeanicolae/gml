%module ialgorithm
%{
#define SWIG_FILE_WITH_INIT
#include "..\..\..\code\GmlLib\inc\IAlgorithm.h";
%}

namespace GML
{
	namespace Algorithm
	{
		class IAlgorithm
		{
		public:
			IAlgorithm();
			%feature("autodoc", "1");
			virtual bool	SetConfiguration(GML::Utils::AttributeList &config) = 0;
			%feature("autodoc", "1");
			virtual bool	GetConfiguration(GML::Utils::AttributeList &config) = 0;
			%feature("autodoc", "1");
			virtual bool	Init() = 0;
			%feature("autodoc", "1");
			virtual void	Execute(UInt32 command)=0;
			%feature("autodoc", "1");
			void	OnExecute(char *command);



		};
	}
}







