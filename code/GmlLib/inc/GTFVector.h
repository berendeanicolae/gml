#ifndef __GTF_VECTOR__
#define __GTF_VECTOR__

#include "Compat.h"
#include "Vector.h"

namespace GML
{
	namespace Utils
	{
		template <class TemplateObject> class GTFVector
		{
			Vector	elements;
		public:
			GTFVector();
			GTFVector(UInt32 initialSize);

			bool						Create(UInt32 initialSize);
			void						Free();

			//------ Insert --------------------------------------------------------------------------
			bool						Push(TemplateObject Element);
			bool						PushFront(TemplateObject Element);
			bool						Insert(TemplateObject Element,UInt32 index);

			bool						PushByRef(TemplateObject &Element);
			bool						PushFrontByRef(TemplateObject &Element);
			bool						InsertByRef(TemplateObject &Element,UInt32 index);

			//------ Get -----------------------------------------------------------------------------
			TemplateObject&				operator [] (UInt32 poz);
			TemplateObject*				GetPtrToObject(UInt32 poz);
			bool						Pop(TemplateObject &Element);

			//------ Delete --------------------------------------------------------------------------
			bool						Delete(UInt32 index);
			bool						DeleteAll();
	
			//------ Info --------------------------------------------------------------------------
			bool						Exists();
			UInt32						GetCount();
			UInt32						Len();
		};		
		template <class TemplateObject> GTFVector<TemplateObject>::GTFVector()
		{
			elements.Create(256,sizeof(TemplateObject));
		}
		template <class TemplateObject> GTFVector<TemplateObject>::GTFVector(UInt32 initialSize)
		{
			elements.Create(initialSize,sizeof(TemplateObject));
		}		
		template <class TemplateObject> bool GTFVector<TemplateObject>::Create(UInt32 initialSize)
		{
			return elements.Create(initialSize,sizeof(TemplateObject));
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
		template <class TemplateObject> bool GTFVector<TemplateObject>::InsertByRef(TemplateObject &Element,UInt32 pos)
		{
			return elements.Insert(&Element,pos);
		}
		template <class TemplateObject> TemplateObject& GTFVector<TemplateObject>::operator [] (UInt32 poz)
		{
			return *(TemplateObject *)elements.Get(poz);
		}
		template <class TemplateObject> TemplateObject*	GTFVector<TemplateObject>::GetPtrToObject(UInt32 poz)
		{
			return (TemplateObject *)elements.Get(poz);
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

	}
}

#endif
