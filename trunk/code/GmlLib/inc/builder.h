#pragma once

#include "compat.h"
#include "INotify.h"
#include "IDataBase.h"


namespace GML
{
	class EXPORT Builder
	{
	public:
		static GML::Utils::INotify*		CreateNotifyer(char *pluginName,void *objectData = NULL);	
		static GML::DB::IDatabase*		CreateDataBase(char *pluginName,GML::Utils::INotify &notify,char *connectionString);
	};
}

