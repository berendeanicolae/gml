#pragma once
#include "IParalelUnit.h"

namespace GML
{
	namespace Utils
	{
		class EXPORT ThreadParalelUnit : public IParalelUnit
		{
			HANDLE			eventWorking,eventWaiting,hThread;
		public:
			ThreadParalelUnit(void);
			~ThreadParalelUnit(void);

			void			DoAction();
			bool			WaitToFinish();
			bool			Execute(unsigned int codeID);
		};
	}
};
