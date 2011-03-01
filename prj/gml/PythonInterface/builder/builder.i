/* File : perceptron.i */
%module builder
%{
#define SWIG_FILE_WITH_INIT
#include "..\..\..\code\GmlLib\inc\Compat.h";
#include "..\..\..\code\GmlLib\inc\builder.h";
%}


namespace GML::Builder
{

	%feature("autodoc", "1");
	static GML::Utils::INotifier*			CreateNotifier(char *pluginName)
	{
		return CreateNotifier(pluginName);
	}

	%feature("autodoc", "1");
	static GML::DB::IDataBase*			CreateDataBase(char *pluginName,GML::Utils::INotifier &notifier)
	{
		return CreateDataBase(pluginName,notifier);
	}

	%feature("autodoc", "1");
	static GML::ML::IConector*			CreateConectors(char *conectorsList,GML::Utils::INotifier &notifier,GML::DB::IDataBase &database)
	{
		return CreateConectors(conectorsList,notifier,database);
	}

	%feature("autodoc", "1");
	static GML::Algorithm::IAlgorithm*	CreateAlgorithm(char *algorithmLib)
	{
		return CreateAlgorithm(algorithmLib);
	}

	//%feature("autodoc", "1");
	//static char*						GetAlgorithmList(char *algorithmLib);											
}






