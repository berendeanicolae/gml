#pragma once

#include "compat.h"
#include "INotify.h"
#include "IDataBase.h"
#include "MLInterface.h"
#include "IAlgorithm.h"


namespace GML
{
	class EXPORT Builder
	{
	public:
		static GML::Utils::INotify*			CreateNotifyer(char *pluginName,void *objectData = NULL);	
		static GML::DB::IDataBase*			CreateDataBase(char *pluginName,GML::Utils::INotify &notify,char *connectionString);
		static GML::ML::IConector*			CreateConectors(char *conectorsList,GML::Utils::INotify &notify,GML::DB::IDataBase &database);
		static GML::Algorithm::IAlgorithm*	CreateAlgorithm(char *algorithmLib,char *algorithmName=NULL);
		static char*						GetAlgorithmList(char *algorithmLib);
	};
}

