#ifndef __I_ALGORITHM__
#define __I_ALGORITHM__

#include "Compat.h"
#include "GMLObject.h"
#include "INotifier.h"
#include "Timer.h"



namespace GML
{
	namespace Algorithm
	{
		class EXPORT IAlgorithm: public GML::Utils::GMLObject
		{
		protected:
			HANDLE						hMainThread;
			bool						StopAlgorithm;		
			GML::Utils::INotifier		*notif;			
			GML::Utils::Timer			algTimer;
			UInt32						Command;

		public:			
			IAlgorithm();

			virtual bool	Init() = 0;
			virtual void	OnExecute()=0;

			bool			Execute(char *command=NULL);
			bool			Wait();
			bool			Wait(UInt32 nrMiliseconds);
		};
	}
}


#endif
