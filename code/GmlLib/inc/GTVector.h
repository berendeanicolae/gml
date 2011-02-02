#ifndef __GTVECTOR_H
#define __GTVECTOR_H

#include "Compat.h"

#define DIR_FORWARD		0	
#define DIR_BACKWARD	1

#define POS_START		0
namespace GML
{
	namespace Utils
	{
		template <class TemplateObject> struct	PTemplateElement
		{
			TemplateObject		*Element;
		};

		template <class TemplateObject> class GTVector
		{
		private:	
			UInt32				ElementsCount;
			UInt32				InitialAlloc;
			UInt32				AlocatedElements;
			UInt8				*Data;
			Int32				(*compFnc)(TemplateObject &element1,TemplateObject &element2);
	
			UInt32				PosToIndex(Int32 index);
			bool				Grow(UInt32 newSize=0);

			// internal functions
			Int32				__Compare(TemplateObject &Element1,TemplateObject &Element2,Int32 (*compFunction)(TemplateObject &element1,TemplateObject &element2));
			void				__QSort(PTemplateElement<TemplateObject> *Data,Int32 (*compFunction)(TemplateObject &element1,TemplateObject &element2),Int32 lo,Int32 hi,bool ascendent);
			Int32				__BinSearch(PTemplateElement<TemplateObject> *Data,TemplateObject& Element,Int32 (*compFunction)(TemplateObject &element1,TemplateObject &element2),Int32 lo,Int32 hi);
		public:
			GTVector(void);
			GTVector(UInt32 alocElements);
			GTVector(const GTVector<TemplateObject> &source);
			~GTVector(void);
	
			//------ Insert --------------------------------------------------------------------------
			bool						Push(TemplateObject Element);
			bool						PushFront(TemplateObject Element);
			bool						Insert(TemplateObject Element,Int32 index);

			bool						PushByRef(TemplateObject &Element);
			bool						PushFrontByRef(TemplateObject &Element);
			bool						InsertByRef(TemplateObject &Element,Int32 index);

			bool						Copy(GTVector<TemplateObject> &vector,Int32 startPos,Int32 endPos);
	
			//------ Get -----------------------------------------------------------------------------
			TemplateObject&				operator [] (Int32 poz);
			TemplateObject*				GetPtrToObject(Int32 poz);
			bool						Pop(TemplateObject &Element);

			//------ Delete --------------------------------------------------------------------------
			bool						Delete(Int32 index);
			bool						DeleteAll();
	
			//------ Info --------------------------------------------------------------------------
			bool						Exists();
			UInt32						GetCount();
			UInt32						Len();

			//------ Operatori -----------------------------------------------------------------------
			GTVector<TemplateObject>&	operator= (GTVector<TemplateObject> &vector);

			//------ FOREACH --------------------------------------------------------------------------
			//void						__ForEachInit(FOREACH_DATA<TemplateObject> &fd);
			//void						__ForEachClose(FOREACH_DATA<TemplateObject> &fd);
			//bool						__ForEachNext(FOREACH_DATA<TemplateObject> &fd);
			//void						__ForEachGet(FOREACH_DATA *fd,int index,void *variable);

	
			//------ Sort --------------------------------------------------------------------------
			void						SetCompareFunction(Int32 (*compFunction)(TemplateObject &element1,TemplateObject &element2));
			void						Sort(bool ascendent=true,Int32 (*compFunction)(TemplateObject &element1,TemplateObject &element2)=NULL);
			bool						BinarySearch(TemplateObject &Element,Int32 *left_location=NULL,Int32 *right_location=NULL,Int32 (*compFunction)(TemplateObject &element1,TemplateObject &element2)=NULL);
			Int32						Find(TemplateObject &Element,Int32 start=POS_START,Int32 direction=DIR_FORWARD,Int32 (*compFunction)(TemplateObject &element1,TemplateObject &element2)=NULL);
			bool						Contains(TemplateObject &Element,Int32 (*compFunction)(TemplateObject &element1,TemplateObject &element2)=NULL);
		};
		#define GTVECTOR_SWAP(e1,e2)	{ aux = e1.Element;e1.Element = e2.Element;e2.Element = aux; }


		template <class TemplateObject> GTVector<TemplateObject>::GTVector(void)
		{
			Data=NULL;
			AlocatedElements=ElementsCount=0;
			InitialAlloc = 32;
			compFnc=NULL;
		}
		template <class TemplateObject> GTVector<TemplateObject>::GTVector(UInt32 allocElements)
		{
			if (allocElements<1) 
				allocElements=1;
			Data=NULL;
			AlocatedElements=ElementsCount=0;
			InitialAlloc = allocElements;
			compFnc=NULL;
		}
		template <class TemplateObject> GTVector<TemplateObject>::GTVector(const GTVector<TemplateObject> &source)
		{
			GTVector<TemplateObject>	*ptr;

			Data=NULL;
			AlocatedElements=ElementsCount=0;
			InitialAlloc = source.ElementsCount;
			if (InitialAlloc<1)
				InitialAlloc = 1;

			ptr = (GTVector<TemplateObject> *)&source;
			ptr->Copy((*this),0,source.ElementsCount);		
			compFnc=NULL;
		}
		template <class TemplateObject> GTVector<TemplateObject>::~GTVector(void)
		{
			DeleteAll();
			if (Data!=NULL)
				free(Data);		
				Data=NULL;
			AlocatedElements=ElementsCount=0;
		}
		template <class TemplateObject> bool GTVector<TemplateObject>::Grow(UInt32 newSize)
		{
			UInt8	*tmp;

			if (newSize==0) 
				newSize=AlocatedElements * 2; 
			else 
				newSize=((newSize/2)+1)*2;

			if ((tmp=(UInt8*)malloc(newSize * sizeof(void *)))==NULL)
				return false;

			if (Data!=NULL)
			{
				memcpy(tmp,Data,sizeof(void *)*ElementsCount);
				free(Data);
			}
			Data = tmp;
			AlocatedElements=newSize;

			return true;
		}
		template <class TemplateObject> UInt32 GTVector<TemplateObject>::PosToIndex(Int32 index)
		{
			if (index>=0)
			{
				if ((UInt32)index<=ElementsCount)
					return (UInt32)index;
				return ElementsCount;
			} else {
				index = -index;
				if ((UInt32)index>ElementsCount)
					return 0;
				return (ElementsCount-index);
			}
		}
		template <class TemplateObject> bool GTVector<TemplateObject>::Push(TemplateObject Element)
		{
			return InsertByRef(Element,ElementsCount);
		}
		template <class TemplateObject> bool GTVector<TemplateObject>::PushByRef(TemplateObject &Element)
		{
			return InsertByRef(Element,ElementsCount);
		}
		template <class TemplateObject> bool GTVector<TemplateObject>::PushFront(TemplateObject Element)
		{
			return InsertByRef(Element,0);
		}
		template <class TemplateObject> bool GTVector<TemplateObject>::PushFrontByRef(TemplateObject &Element)
		{
			return InsertByRef(Element,0);
		}
		template <class TemplateObject> bool GTVector<TemplateObject>::Insert(TemplateObject Element,Int32 pos)
		{
			return InsertByRef(Element,pos);
		}
		template <class TemplateObject> bool GTVector<TemplateObject>::InsertByRef(TemplateObject &Element,Int32 pos)
		{
			UInt32						newAllocSize,index;
			PTemplateElement<TemplateObject>	*ptr;

			if (ElementsCount+1>=AlocatedElements)
			{
				if (AlocatedElements==0)
					newAllocSize = InitialAlloc;
				else
					newAllocSize = AlocatedElements * 2;
				if (Grow(newAllocSize)==false)
					return false;
			}
			index = PosToIndex(pos);
			if (index<ElementsCount)
			{
				memmove(&Data[(index+1)*sizeof(void*)],&Data[index*sizeof(void*)],(ElementsCount-index)*sizeof(void*));
			} else {
				index=ElementsCount;
			}
			ptr = (PTemplateElement<TemplateObject> *)&Data[index*sizeof(void*)];
			ptr->Element = new TemplateObject(Element);
			ElementsCount++;
			return true;
		}

		template <class TemplateObject> bool GTVector<TemplateObject>::Copy(GTVector<TemplateObject> &dest,Int32 startPos,Int32 endPos)
		{
			PTemplateElement<TemplateObject>	*ptr;
			UInt32						start,end,index;

			if (Data==NULL)
				return false;
			if (ElementsCount==0)
				return true;
			start = PosToIndex(startPos);
			end = PosToIndex(endPos);

			if ((start==end) && (start==ElementsCount))
				return true;
			if (end==ElementsCount)
				end--;
			do
			{
				if (start==ElementsCount)
					start = 0;
				ptr = (PTemplateElement<TemplateObject> *)&Data[start*sizeof(void*)];
				if (dest.PushByRef(*(ptr->Element))==false)
					return false;
				if (start==end)
					break;
				start++;
			} while (true);
			return true;
		}
		template <class TemplateObject> TemplateObject& GTVector<TemplateObject>::operator [] (Int32 poz)
		{
			UInt32						index;
			PTemplateElement<TemplateObject>	*ptr;

			if ((ElementsCount==0) || (Data==NULL)) 
			{
				TemplateObject	tmpObj;
				return tmpObj;
			}
			index = PosToIndex(poz);
			ptr = (PTemplateElement<TemplateObject> *)&Data[index*sizeof(void*)];
			return *(ptr->Element);
		}
		template <class TemplateObject> TemplateObject*	GTVector<TemplateObject>::GetPtrToObject(Int32 poz)
		{
			UInt32						index;
			PTemplateElement<TemplateObject>	*ptr;

			if ((ElementsCount==0) || (Data==NULL)) 
				return NULL;
			index = PosToIndex(poz);
			ptr = (PTemplateElement<TemplateObject> *)&Data[index*sizeof(void*)];
			return ptr->Element;
		}
		template <class TemplateObject> bool GTVector<TemplateObject>::Pop(TemplateObject &Element)
		{
			PTemplateElement<TemplateObject>	*ptr;

			if ((Data==NULL) || (ElementsCount==0))
				return false;

			ptr = (PTemplateElement<TemplateObject> *)&Data[(ElementsCount-1)*sizeof(void*)];
			Element = *ptr->Element;

			return Delete(ElementsCount-1);
		}

		template <class TemplateObject> bool GTVector<TemplateObject>::Delete(Int32 poz)
		{
			UInt32						index;
			PTemplateElement<TemplateObject>	*ptr;

			if ((Data==NULL) || (ElementsCount==0))
				return false;
			index = PosToIndex(poz);
			ptr = (PTemplateElement<TemplateObject> *)&Data[index*sizeof(void*)];
			if (ptr->Element!=NULL)
				free(ptr->Element);
			ptr->Element=NULL;
			memmove(&Data[index*sizeof(void*)],&Data[(index+1)*sizeof(void*)],(ElementsCount-(index+1))*sizeof(void*));
			ElementsCount--;
			return true;
		}
		template <class TemplateObject> bool GTVector<TemplateObject>::DeleteAll()
		{
			UInt32						index;
			PTemplateElement<TemplateObject>	*ptr;

			if ((Data==NULL) || (ElementsCount==0))
				return true;

			for (index=0;index<ElementsCount;index++)
			{
				ptr = (PTemplateElement<TemplateObject> *)&Data[index*sizeof(void*)];
				if (ptr->Element!=NULL)
					delete ptr->Element;
			}
			ElementsCount=0;
			return true;
		}
		template <class TemplateObject> bool GTVector<TemplateObject>::Exists()
		{
			return (Data!=NULL);
		}
		template <class TemplateObject> UInt32 GTVector<TemplateObject>::GetCount()
		{
			return ElementsCount;
		}
		template <class TemplateObject> UInt32 GTVector<TemplateObject>::Len()
		{
			return ElementsCount;
		}
		template <class TemplateObject> GTVector<TemplateObject>& GTVector<TemplateObject>::operator = (GTVector<TemplateObject> &vector)
		{
			if (DeleteAll())
			{
				vector.Copy((*this),0,vector.ElementsCount);		
			}
			return *this;	
		}
		template <class TemplateObject> void   GTVector<TemplateObject>::SetCompareFunction(Int32 (*compFunction)(TemplateObject &element1,TemplateObject &element2))
		{
			compFnc = compFunction;
		}
		template <class TemplateObject> Int32  GTVector<TemplateObject>::__Compare(TemplateObject &Element1,TemplateObject &Element2,Int32 (*compFunction)(TemplateObject &element1,TemplateObject &element2))
		{
			if (compFunction!=NULL)
			{
				return compFunction(Element1,Element2);
			} else {
				if (Element1<Element2)
					return -1;
				if (Element1>Element2)
					return 1;
				return 0;
			}	
		}		
		template <class TemplateObject> void   GTVector<TemplateObject>::__QSort(PTemplateElement<TemplateObject> *pData,Int32 (*compFunction)(TemplateObject &element1,TemplateObject &element2),Int32 lo,Int32 hi,bool ascendent)
		{
			Int32								left,right,mid;
			PTemplateElement<TemplateObject>	*pivot;
			TemplateObject						*aux;

			if (lo>=hi) 
				return;

			left=lo;
			right=hi;
			mid=(lo+hi)/2;
			GTVECTOR_SWAP(pData[left],pData[mid]);
			pivot=&pData[left];
			lo++;

			while (lo<=hi)
			{
				if (ascendent)
				{
					while ((lo<=right) && (__Compare(*(pData[lo].Element),*pivot->Element,compFunction)<=0))
						++lo;
					while ((hi>=left) &&  (__Compare(*(pData[hi].Element),*pivot->Element,compFunction)>0))
						--hi;
				} else {
					while ((lo<=right) && (__Compare(*(pData[lo].Element),*pivot->Element,compFunction)>=0))
						++lo;
					while ((hi>=left) &&  (__Compare(*(pData[hi].Element),*pivot->Element,compFunction)<0))
						--hi;
				}
				if (lo<hi)
				{
					GTVECTOR_SWAP(pData[lo],pData[hi]);

				}
			} 
			GTVECTOR_SWAP(pData[left],pData[hi]);

			// apelurile recursive
			__QSort(pData,compFunction,left,hi-1,ascendent);
			__QSort(pData,compFunction,hi+1,right,ascendent);
		}
		template <class TemplateObject> Int32  GTVector<TemplateObject>::__BinSearch(PTemplateElement<TemplateObject> *Data,TemplateObject& Element,Int32 (*compFunction)(TemplateObject &element1,TemplateObject &element2),Int32 lo,Int32 hi)
		{
			Int32 mij;
			Int32 res;

			do
			{
				mij=(lo+hi)/2;
				if ((res=__Compare(Element,*Data[mij].Element,compFunction))==0) 
					return mij;
				if (res<0) hi=mij-1;
				if (res>0) lo=mij+1;
			} while ((lo<=hi) && (hi>=0));
			return -1;
		}
		template <class TemplateObject> void   GTVector<TemplateObject>::Sort(bool ascendent,Int32 (*compFunction)(TemplateObject &element1,TemplateObject &element2))
		{
			if ((Data==NULL) || (ElementsCount==0)) 
				return;
			if (compFunction)
				__QSort((PTemplateElement<TemplateObject> *)Data,compFunction,0,ElementsCount-1,ascendent);
			else
				__QSort((PTemplateElement<TemplateObject> *)Data,compFnc,0,ElementsCount-1,ascendent);
		}
		template <class TemplateObject> bool   GTVector<TemplateObject>::BinarySearch(TemplateObject &Element, Int32 *left_location, Int32 *right_location,Int32 (*compFunctionToUse)(TemplateObject &element1,TemplateObject &element2))
		{
			Int32								poz;
			PTemplateElement<TemplateObject>	*ptrData;
			Int32 (*myCompFunction)(TemplateObject &element1,TemplateObject &element2) = NULL;

			if (compFunctionToUse!=NULL)
				myCompFunction = compFunctionToUse;
			else
				myCompFunction = compFnc;

			ptrData = (PTemplateElement<TemplateObject> *)Data;

			if ((Data==NULL) || (ElementsCount==0))
				return false;
			if ((poz=__BinSearch(ptrData,Element,myCompFunction,0,ElementsCount-1))==-1)
				return false;
			// merg in stanga daca e nevoie
			if (left_location!=NULL)
			{
				*left_location = poz-1;
				while (((*left_location)>=0) && (__Compare(*(ptrData[(*left_location)].Element),Element,myCompFunction)==0))
					(*left_location)--;
				(*left_location)++;
			}
			// merg in dreapta daca e nevoie
			if (right_location!=NULL)
			{
				*right_location = poz+1;
				while (((*right_location)<(Int32)ElementsCount) && (__Compare(*(ptrData[(*right_location)].Element),Element,myCompFunction)==0))
					(*right_location)++;
				(*right_location)--;
			}
			return true;
		}
		template <class TemplateObject> Int32  GTVector<TemplateObject>::Find(TemplateObject &Element, Int32 start, Int32 direction,Int32 (*compFunctionToUse)(TemplateObject &element1,TemplateObject &element2))
		{
			PTemplateElement<TemplateObject>	*ptrData;
			Int32 (*myCompFunction)(TemplateObject &element1,TemplateObject &element2) = NULL;

			if (compFunctionToUse!=NULL)
				myCompFunction = compFunctionToUse;
			else
				myCompFunction = compFnc;

			if ((Data==NULL) || (ElementsCount==0))
				return -1;
			ptrData = (PTemplateElement<TemplateObject> *)Data;

			if (direction==DIR_FORWARD)
			{
				if (start<0) start=0;
				while ((start<ElementsCount))
				{			
					if (__Compare(*(ptrData[start].Element),Element,myCompFunction)==0) 
						return start;
					start++;
				}
			} 
			if (direction==DIR_BACKWARD)
			{
				if ((start<0) || (start>=ElementsCount)) start=ElementsCount-1;
				while ((start>=0))
				{			
					if (__Compare(*(ptrData[start].Element),Element,myCompFunction)==0) 
						return start;
					start--;
				}
			}
			return -1;
		}
		template <class TemplateObject> bool   GTVector<TemplateObject>::Contains(TemplateObject &Element,Int32 (*compFunctionToUse)(TemplateObject &element1,TemplateObject &element2))
		{
			return (Find(Element,POS_START,DIR_FORWARD,compFunctionToUse)>=0);
		}
	}
}
#endif