#ifndef __INTERFACE_ML_ALGORITHM__
#define __INTERFACE_ML_ALGORITHM__

#include "IAlgorithm.h"
#include "GString.h"
#include "IDataBase.h"
#include "MLInterface.h"
#include "Builder.h"

namespace GML
{
	namespace Algorithm
	{
		class EXPORT IMLAlgorithm: public GML::Algorithm::IAlgorithm
		{
		protected:
			UInt32					threadsCount;
			GML::Utils::GString		Conector;
			GML::Utils::GString		DataBase;
			GML::Utils::GString		Notifier;
			GML::DB::IDataBase		*db;
			GML::ML::IConnector		*con;

			bool					InitConnections();

		public:
			IMLAlgorithm();	
		};
	}
}

#endif
