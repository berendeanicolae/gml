#pragma once

#include "compat.h"
#include "INotifier.h"
#include "IDataBase.h"
#include "IConnector.h"
#include "IAlgorithm.h"
#include "TemplateParser.h"
#include "GString.h"


namespace GML
{
	class EXPORT Builder
	{
	public:
		static GML::Utils::INotifier*		CreateNotifier(char *buildString);	
		static GML::DB::IDataBase*			CreateDataBase(char *buildString,GML::Utils::INotifier &notify);
		static GML::ML::IConnector*			CreateConnector(char *buildString,GML::Utils::INotifier &notify);
		static GML::Algorithm::IAlgorithm*	CreateAlgorithm(char *algorithmLib);	
		static bool							GetPluginProperties(char *pluginName,GML::Utils::AttributeList &attr,GML::Utils::GString *fullName=NULL);
	};
}

