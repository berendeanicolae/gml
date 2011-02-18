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
		protected:
			GML::Utils::INotify									*notif;
			GML::Utils::GTFVector<GML::Utils::AttributeLink>	AttrLinks;
			HANDLE												hMainThread;

			bool			AddString(char *Name,GML::Utils::GString &LocalAddr,char *defaultValue,char *Description=NULL);
			bool			AddBool(char *Name,bool &LocalAddr,bool defaultValue,char *Description=NULL);
			bool			AddDouble(char *Name,double &LocalAddr,double defaultValue,char *Description=NULL);
			bool			AddUInt32(char *Name,UInt32 &LocalAddr,UInt32 defaultValue,char *Description=NULL);
			bool			AddInt32(char *Name,Int32 &LocalAddr,Int32 defaultValue,char *Description=NULL);
		public:			
			IAlgorithm();

			bool			SetConfiguration(GML::Utils::AttributeList &config);
			bool			GetConfiguration(GML::Utils::AttributeList &config);

			virtual bool	Init() = 0;
			virtual void	OnExecute(char* command)=0;

			// void			Execute(char *command);
			virtual void			Execute(UInt32 command)=0;

		};
	}
}


#endif
