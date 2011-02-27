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
		static GML::Utils::INotify*			CreateNotifyer(char *pluginName);	
		static GML::DB::IDataBase*			CreateDataBase(char *pluginName,GML::Utils::INotify &notify);
		static GML::ML::IConector*			CreateConectors(char *conectorsList,GML::Utils::INotify &notify,GML::DB::IDataBase &database);
		static GML::Algorithm::IAlgorithm*	CreateAlgorithm(char *algorithmLib);		
	};
}

