/* File : perceptron.i */
%module builder
%{
#define SWIG_FILE_WITH_INIT
#include "..\..\..\code\GmlLib\inc\Compat.h";
#include "..\..\..\code\GmlLib\inc\builder.h";
%}

namespace GML{
class Builder
	{
	public:
		%feature("autodoc", "1");
		static GML::Utils::INotify*			CreateNotifyer(char *pluginName);

		%feature("autodoc", "1");
		static GML::DB::IDataBase*			CreateDataBase(char *pluginName,GML::Utils::INotify &notify);

		%feature("autodoc", "1");
		static GML::ML::IConector*			CreateConectors(char *conectorsList,GML::Utils::INotify &notify,GML::DB::IDataBase &database);

		%feature("autodoc", "1");
		static GML::Algorithm::IAlgorithm*	CreateAlgorithm(char *algorithmLib);

		%feature("autodoc", "1");
		static char*						GetAlgorithmList(char *algorithmLib);											
	};
}







