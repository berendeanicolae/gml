#pragma once

#include "compat.h"
#include "INotifier.h"
#include "IDataBase.h"
#include "MLInterface.h"
#include "IAlgorithm.h"


namespace GML
{
	class EXPORT Builder
	{
	public:
		static GML::Utils::INotifier*		CreateNotifier(char *pluginName);	
		static GML::DB::IDataBase*			CreateDataBase(char *pluginName,GML::Utils::INotifier &notify);
		static GML::ML::IConnector*			CreateConnectors(char *conectorsList,GML::Utils::INotifier &notify,GML::DB::IDataBase &database);
		static GML::Algorithm::IAlgorithm*	CreateAlgorithm(char *algorithmLib);		
	};
}

