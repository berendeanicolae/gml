#ifndef __INTERFACE_ML_ALGORITHM__
#define __INTERFACE_ML_ALGORITHM__

#include "IAlgorithm.h"
#include "GString.h"
#include "IDataBase.h"
#include "MLInterface.h"
#include "Builder.h"
#include "ThreadParalelUnit.h"
#include "AlgorithmResult.h"
#include "Indexes.h"
#include "BitSet.h"

namespace GML
{
	namespace Algorithm
	{
		enum {
			HASH_FILE_TEXT = 0,
			HASH_FILE_BINARY		
		};
		struct EXPORT MLThreadData
		{
			UInt32							ThreadID;
			GML::ML::MLRecord				Record;
			GML::Utils::AlgorithmResult		Res;
			GML::Utils::Interval			Range;
			void*							Context;
		};
		class EXPORT IMLAlgorithm: public GML::Algorithm::IAlgorithm
		{
		public:
			MLThreadData					*ThData;

		protected:
			// properties
			UInt32							threadsCount;
			UInt32							HashFileType;
			GML::Utils::GString				Conector;
			GML::Utils::GString				DataBase;
			GML::Utils::GString				Notifier;
			GML::Utils::GString				HashFileName;	
			GML::DB::IDataBase				*db;
			GML::ML::IConnector				*con;

			// local variables
			GML::Utils::ThreadParalelUnit	*tpu;
			

			void							AddHashSavePropery();

			bool							InitConnections();
			bool							InitThreads();
			bool							ExecuteParalelCommand(UInt32 command);
			bool							SplitMLThreadDataRange(UInt32 maxCount);
			bool							SaveHashResult(char *fname,UInt32 method,GML::Utils::BitSet &bs,UInt32 CacheSize=0x1000);
			bool							SaveHashResult(char *fname,UInt32 method,GML::Utils::GTFVector<UInt8> &v,UInt32 CacheSize=0x1000);

			virtual bool					OnInitThreadData(GML::Algorithm::MLThreadData &thData);
			virtual bool					OnInitThreads();

		public:
			IMLAlgorithm();	
			virtual void					OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);

		};
	}
}

#endif
