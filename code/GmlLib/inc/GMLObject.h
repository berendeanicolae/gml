#ifndef __GML_OBJECT__
#define __GML_OBJECT__

#include "compat.h"
#include "AttributeList.h"
#include "GTFVector.h"

namespace GML
{
	namespace Utils
	{
		class EXPORT GMLObject
		{
			GML::Utils::GTFVector<GML::Utils::AttributeLink>	AttrLinks;
		protected:
			char*												ObjectName;
		protected:
			bool			LinkPropertyToString(char *Name,GML::Utils::GString &LocalAddr,char *defaultValue,char *Description=NULL);
			bool			LinkPropertyToBool(char *Name,bool &LocalAddr,bool defaultValue,char *Description=NULL);
			bool			LinkPropertyToDouble(char *Name,double &LocalAddr,double defaultValue,char *Description=NULL);
			bool			LinkPropertyToUInt32(char *Name,UInt32 &LocalAddr,UInt32 defaultValue,char *Description=NULL);
			bool			LinkPropertyToInt32(char *Name,Int32 &LocalAddr,Int32 defaultValue,char *Description=NULL);
			bool			RemoveProperty(char *Name);
			
		
		public:
			bool			SetProperty(GML::Utils::AttributeList &config);
			bool			SetProperty(char *config);
			bool			SetPropertyFromFile(char *fileName);
			
			bool			GetProperty(GML::Utils::AttributeList &config);
			bool			SavePropery(char *fileName);
			char*			GetObjectName();
		public:
			GMLObject();
		};
	}
}

#endif

