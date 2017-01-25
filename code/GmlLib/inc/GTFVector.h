#ifndef __GTF_VECTOR__
#define __GTF_VECTOR__

#include "Compat.h"
#include "Vector.h"

typedef int					(*_BinarySearchCompFunction)(void *element1,void *element2);

namespace GML
{
	namespace Utils
	{
		template <class TemplateObject> class GTFVector
		{
			Vector	elements;
		public:
			GTFVector();
			GTFVector(UInt32 initialSize,bool resizeToInitialSize);

			bool						Create(UInt32 initialSize,bool resizeToInitialSize=false);
			void						Free();

			//------ Insert --------------------------------------------------------------------------
			bool						Push(TemplateObject Element);
			bool						PushFront(TemplateObject Element);
			bool						Insert(TemplateObject Element,UInt32 index);
			bool						Insert(TemplateObject Element,int (*_CmpFunc)(TemplateObject &e1,TemplateObject &e2),bool ascendent=true);

			bool						PushByRef(TemplateObject &Element);
			bool						PushFrontByRef(TemplateObject &Element);
			bool						InsertByRef(TemplateObject &Element,UInt32 index);
			bool						InsertByRef(TemplateObject &Element,int (*_CmpFunc)(TemplateObject &e1,TemplateObject &e2),bool ascendent=true);

			//------ Get -----------------------------------------------------------------------------
			TemplateObject&				operator [] (UInt32 poz);
			TemplateObject*				GetPtrToObject(UInt32 poz);
			TemplateObject*				GetVector();						
			bool						Pop(TemplateObject &Element);

			//------ Delete --------------------------------------------------------------------------
			bool						Delete(UInt32 index);
			bool						DeleteAll();

			//------ Soft & Find --------------------------------------------------------------------------
			void						Sort(int (*_CmpFunc)(TemplateObject &e1,TemplateObject &e2),bool ascendent=true);
			int							BinarySearch(TemplateObject &element,int (*_CmpFunc)(TemplateObject &e1,TemplateObject &e2));
			int							EqualRange(TemplateObject &element,int (*_CmpFunc)(TemplateObject &e1,TemplateObject &e2),int *left_location,int *right_location);

			//------ Info --------------------------------------------------------------------------
			bool						Exists();
			bool						Resize(UInt32 newSize);
			UInt32						GetCount();
			UInt32						Len();
		};		
		template <class TemplateObject> GTFVector<TemplateObject>::GTFVector()
		{
			elements.Create(256,sizeof(TemplateObject));
		}
		template <class TemplateObject> GTFVector<TemplateObject>::GTFVector(UInt32 initialSize,bool resizeToAllocElements)
		{
			elements.Create(initialSize,sizeof(TemplateObject),resizeToAllocElements);
		}		
		template <class TemplateObject> bool GTFVector<TemplateObject>::Create(UInt32 initialSize,bool resizeToAllocElements)
		{
			return elements.Create(initialSize,sizeof(TemplateObject),resizeToAllocElements);
		}
		template <class TemplateObject> void GTFVector<TemplateObject>::Free()
		{
			elements.Free();
		}
		template <class TemplateObject> bool GTFVector<TemplateObject>::Push(TemplateObject Element)
		{
			return elements.Push(&Element);
		}
		template <class TemplateObject> bool GTFVector<TemplateObject>::PushByRef(TemplateObject &Element)
		{
			return elements.Push(&Element);
		}
		template <class TemplateObject> bool GTFVector<TemplateObject>::PushFront(TemplateObject Element)
		{
			return elements.Insert(&Element,0);
		}
		template <class TemplateObject> bool GTFVector<TemplateObject>::PushFrontByRef(TemplateObject &Element)
		{
			return elements.Insert(&Element,0);
		}
		template <class TemplateObject> bool GTFVector<TemplateObject>::Insert(TemplateObject Element,UInt32 pos)
		{
			return elements.Insert(&Element,pos);
		}
		template <class TemplateObject> bool GTFVector<TemplateObject>::Insert(TemplateObject Element,int (*_CmpFunc)(TemplateObject &e1,TemplateObject &e2),bool ascendent)
		{
			return elements.Insert(&Element,(_BinarySearchCompFunction)_CmpFunc,ascendent);
		}
		template <class TemplateObject> bool GTFVector<TemplateObject>::InsertByRef(TemplateObject &Element,UInt32 pos)
		{
			return elements.Insert(&Element,pos);
		}
		template <class TemplateObject> bool GTFVector<TemplateObject>::InsertByRef(TemplateObject &Element,int (*_CmpFunc)(TemplateObject &e1,TemplateObject &e2),bool ascendent)
		{
			return elements.Insert(&Element,(_BinarySearchCompFunction)_CmpFunc,ascendent);
		}
		template <class TemplateObject> TemplateObject& GTFVector<TemplateObject>::operator [] (UInt32 poz)
		{
			return *(TemplateObject *)elements.Get(poz);
		}
		template <class TemplateObject> TemplateObject*	GTFVector<TemplateObject>::GetPtrToObject(UInt32 poz)
		{
			return (TemplateObject *)elements.Get(poz);
		}
		template <class TemplateObject> TemplateObject*	GTFVector<TemplateObject>::GetVector()
		{
			return (TemplateObject *)elements.GetVector();
		}
		template <class TemplateObject> bool GTFVector<TemplateObject>::Pop(TemplateObject &Element)
		{
			if (elements.GetSize()==0)
				return false;
			if (elements.CopyElement(elements.GetSize()-1,&Element)==false)
				return false;
			return elements.Delete(elements.GetSize()-1);
		}

		template <class TemplateObject> bool GTFVector<TemplateObject>::Delete(UInt32 poz)
		{
			return elements.Delete(poz);
		}
		template <class TemplateObject> bool GTFVector<TemplateObject>::DeleteAll()
		{
			return elements.DeleteAll();
		}
		template <class TemplateObject> bool GTFVector<TemplateObject>::Exists()
		{
			return elements.Exists();
		}
		template <class TemplateObject> UInt32 GTFVector<TemplateObject>::Len()
		{
			return elements.GetSize();
		}
		template <class TemplateObject> UInt32 GTFVector<TemplateObject>::GetCount()
		{
			return elements.GetSize();
		}
		template <class TemplateObject> bool  GTFVector<TemplateObject>::Resize(UInt32 newSize)
		{
			return elements.Resize(newSize);
		}
		template <class TemplateObject> void  GTFVector<TemplateObject>::Sort(int (*_CmpFunc)(TemplateObject &e1,TemplateObject &e2),bool ascendent)
		{
			elements.Sort((_BinarySearchCompFunction)_CmpFunc,ascendent);
		}
		template <class TemplateObject> int	 GTFVector<TemplateObject>::BinarySearch(TemplateObject &element,int (*_CmpFunc)(TemplateObject &e1,TemplateObject &e2))
		{
			return elements.BinarySearch(&element,(_BinarySearchCompFunction)_CmpFunc);
		}
		template <class TemplateObject> int	 GTFVector<TemplateObject>::EqualRange(TemplateObject &element,int (*_CmpFunc)(TemplateObject &e1,TemplateObject &e2),int *left_location,int *right_location)
		{
			return elements.EqualRange(&element,(_BinarySearchCompFunction)_CmpFunc,left_location,right_location);
		}
	}
}

#endif
