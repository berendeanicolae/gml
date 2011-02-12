#pragma once

#include "Compat.h"

typedef int					(*_BinarySearchCompFunction)(void *element1,void *element2);

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
	
			bool			Create(unsigned int alocElements=32,unsigned int ElemSize=sizeof(void *)); 
			void			Free();
			bool			Exists() { return (Data!=NULL); }

			unsigned int	GetSize() { return NrElemente; }
			unsigned int	GetElementSize() { return ElementSize; }
			void*			GetVector() { return Data; }
			void*			Get(unsigned int index);
			int				BinarySearch(void *Element,_BinarySearchCompFunction cmpFunc);

			bool			Push(void *Element);
			bool			Insert(void *Element,unsigned int index);
			bool			CopyElement(unsigned int index,void *addr);
			bool			Delete(unsigned int index);
			bool			DeleteAll();
			bool			Resize(unsigned int newSize);
	
		};
	}
}