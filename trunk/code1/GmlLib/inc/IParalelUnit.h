#pragma once

#include "compat.h"

namespace GML
{
	namespace Utils
	{
		class EXPORT IParalelUnit
		{
		protected:
			unsigned int	ID;
			unsigned int	codeID;
			void*			context;
			void			(*fnThread) (IParalelUnit *,void *);

		public:
			bool			Init(unsigned int ID,void *context,void (*fnThread) (IParalelUnit *,void *));
			virtual bool	WaitToFinish()=0;
			virtual bool	Execute(unsigned int codeID)=0;
			unsigned int	GetID();
			unsigned int	GetCodeID();
		
		};
	}
}
