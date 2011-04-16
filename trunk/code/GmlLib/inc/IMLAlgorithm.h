#ifndef __INTERFACE_ML_ALGORITHM__
#define __INTERFACE_ML_ALGORITHM__

#include "IAlgorithm.h"
#include "GString.h"
#include "IDataBase.h"
#include "MLInterface.h"
#include "Builder.h"
#include "ThreadParalelUnit.h"

namespace GML
{
	namespace Algorithm
	{
		class EXPORT IMLAlgorithm: public GML::Algorithm::IAlgorithm
		{
		protected:
			// properties
			UInt32							threadsCount;
			GML::Utils::GString				Conector;
			GML::Utils::GString				DataBase;
			GML::Utils::GString				Notifier;
			GML::DB::IDataBase				*db;
			GML::ML::IConnector				*con;

			// local variables
			GML::Utils::ThreadParalelUnit	*tpu;

			bool							InitConnections();
			bool							InitThreads();
			bool							ExecuteParalelCommand(UInt32 command);

		public:
			IMLAlgorithm();	
			virtual void					OnRunThreadCommand(UInt32 threadID,UInt32 threadCommand);
		};
	}
}

#endif
