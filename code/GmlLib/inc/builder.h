#ifndef __BUILDER__H__
#define __BUILDER__H__

#include "Compat.h"
#include "GString.h"
#include "INotify.h"

namespace GML
{
	class EXPORT Builder
	{
	public:
		static GML::Utils::INotify* CreateNotifyer(char *pluginName,void *objectData = NULL);		
	};
}

#endif
