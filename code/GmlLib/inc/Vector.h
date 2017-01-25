#pragma once

#include "Compat.h"

typedef int					(*_BinarySearchCompFunction)(void *element1,void *element2);
typedef int					(*_ContextCompFunction)(void *element1,void *element2,void *context);

namespace GML
{
	namespace Utils
	{
		class EXPORT Vector
		{
		private:
			unsigned int	NrElemente;
			unsigned int	ElementSize;
			unsigned int	AlocatedElements;
			unsigned char	*Data;

			bool			Grow(unsigned int newSize=0xFFFFFFFF);
		public:
			Vector(void);
			~Vector(void);

			void			operator =(Vector& v);
	
			bool			Create(unsigned int alocElements=32,unsigned int ElemSize=sizeof(void *),bool resizeToAllocElements=false); 
			void			Free();
			bool			Exists() { return (Data!=NULL); }

			unsigned int	GetSize() { return NrElemente; }
			unsigned int	GetElementSize() { return ElementSize; }
			void*			GetVector() { return Data; }
			void*			Get(unsigned int index);

			int				BinarySearch(void *Element,_BinarySearchCompFunction cmpFunc);
			bool			EqualRange(void *Element,_BinarySearchCompFunction cmpFunc,int *left_location,int *right_location);
			void			Sort(_BinarySearchCompFunction cmpFunc,bool ascendet);
			void			Sort(_ContextCompFunction cmpFunc,bool ascendet,void *Context);

			bool			Push(void *Element);
			bool			Insert(void *Element,unsigned int index);
			bool			Insert(void *Element,_BinarySearchCompFunction cmpFunc,bool ascendent);
			bool			CopyElement(unsigned int index,void *addr);
			bool			Delete(unsigned int index);
			bool			DeleteAll();
			bool			Resize(unsigned int newSize);
			
		};
	}
}
