#ifndef __I_ALGORITHM__
#define __I_ALGORITHM__

#include "Compat.h"
#include "AttributeList.h"
#include "INotify.h"
#include "GTFVector.h"


namespace GML
{
	namespace Algorithm
	{
		class EXPORT IAlgorithm
		{
			GML::Utils::GTFVector<GML::Utils::AttributeLink>	AttrLinks;
			HANDLE												hMainThread;
		protected:
			GML::Utils::INotify									*notif;

			bool			LinkString(char *Name,GML::Utils::GString &LocalAddr,char *defaultValue,char *Description=NULL);
			bool			LinkBool(char *Name,bool &LocalAddr,bool defaultValue,char *Description=NULL);
			bool			LinkDouble(char *Name,double &LocalAddr,double defaultValue,char *Description=NULL);
			bool			LinkUInt32(char *Name,UInt32 &LocalAddr,UInt32 defaultValue,char *Description=NULL);
			bool			LinkInt32(char *Name,Int32 &LocalAddr,Int32 defaultValue,char *Description=NULL);
		public:			
			IAlgorithm();

			bool			SetConfiguration(GML::Utils::AttributeList &config);
			bool			SetConfiguration(char *config);
			bool			GetConfiguration(GML::Utils::AttributeList &config);

			virtual bool	Init() = 0;
			virtual void	OnExecute(char* command)=0;

			bool			Execute(char *command);
			bool			Wait();
			bool			Wait(UInt32 nrMiliseconds);

		};
	}
}


#endif
