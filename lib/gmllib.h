#ifndef __GML_LIB__
#define __GML_LIB__

#ifndef __COMPAT_H
#define __COMPAT_H

#define NOTIFYER_EXT		".ntf"
#define DATABASE_EXT		".db"
#define CONNECTOR_EXT		".dbc"
#define ALGORITHM_EXT		".alg"

#define NOTIFYER_FOLDER		"Notifiers"
#define DATABASE_FOLDER		"DataBases"
#define CONNECTOR_FOLDER	"Connectors"
#define ALGORITHM_FOLDER	"Algorithms"

#ifdef _DEBUG
	#define DEBUGMSG	printf
#else
	#define DEBUGMSG
#endif

#ifdef WIN32
	#define OS_WINDOWS
#endif

#ifdef OS_WINDOWS
    #ifdef OS_UNIX
        #undef OS_UNIX
    #endif
#endif

#ifdef OS_WINDOWS
	#include "windows.h"
	#include <string>

	#define UInt8				unsigned char
	#define UInt16				unsigned short
	#define UInt32				unsigned int
	#define UInt64				unsigned __int64

	#define Int8				char
	#define Int16				short
	#define Int32				int
	#define Int64				__int64

	#define FILE_HANDLE			HANDLE

	#define memcpy				memcpy
	#define memmove				memmove
	#define memcmp				memcmp
	#define free				free
	#define MEMCOPY				memcpy
	#define MEMSET				memset

	#define EXPORT				_declspec(dllexport)

#endif

#ifdef OS_UNIX
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <stdint.h>
    #include <stddef.h>
    #include <unistd.h>

	#define UInt8				uint8_t
	#define UInt16				uint16_t
	#define UInt32				uint32_t
	#define UInt64				uint64_t

	#define FILE_HANDLE				int
	#define INVALID_HANDLE_VALUE	-1
	#define GENERIC_ERROR			-1

	#define memcpy				memcpy
	#define memmove				memmove
	#define memcmp				memcmp
	#define free				free
#endif

#define LIB_INTERFACE(tip,author,version,description)\
	BOOL APIENTRY DllMain( HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved) { return TRUE; }; \
	extern "C" EXPORT void* CreateInterface() { return new tip (); };\
	extern "C" EXPORT char*	GetInterfaceAuthor() { return (char*)author; };\
	extern "C" EXPORT char*	GetInterfaceDescription() { return (char*)description; };\
	extern "C" EXPORT int	GetInterfaceVersion() { return (int)version; };\
	extern "C" EXPORT bool	GetInterfaceProperty(GML::Utils::AttributeList &attr)\
	{														\
		tip* t = new tip();									\
		if (t!=NULL)										\
			return t->GetProperty(attr);					\
		return false;										\
	};														\
	extern "C" EXPORT bool	SaveInterfaceTemplate(char* fileName)	\
	{																\
		GML::Utils::AttributeList attr;								\
		tip* t = new tip();											\
		if (t!=NULL) {												\
			if (t->GetProperty(attr))								\
				return attr.Save(fileName);							\
			}														\
		return false;												\
	};															
	

#endif

//-----------------------------------------------------------------------------------------------------------------------

//===================== GTVector.h =================================================================================
#ifndef __GTVECTOR_H
#define __GTVECTOR_H


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
			Int32				(*compFnc)(TemplateObject &element1,TemplateObject &element2,void *context);
	
			UInt32				PosToIndex(Int32 index);
			bool				Grow(UInt32 newSize=0);

			// internal functions
			Int32				__Compare(TemplateObject &Element1,TemplateObject &Element2,Int32 (*compFunction)(TemplateObject &element1,TemplateObject &element2,void *context),void *context);
			void				__QSort(PTemplateElement<TemplateObject> *Data,Int32 (*compFunction)(TemplateObject &element1,TemplateObject &element2,void *context),Int32 lo,Int32 hi,bool ascendent,void *context);
			Int32				__BinSearch(PTemplateElement<TemplateObject> *Data,TemplateObject& Element,Int32 (*compFunction)(TemplateObject &element1,TemplateObject &element2,void *context),Int32 lo,Int32 hi,void *context);
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
			void						Sort(bool ascendent=true,Int32 (*compFunction)(TemplateObject &element1,TemplateObject &element2,void *context)=NULL,void *context=NULL);			
			bool						BinarySearch(TemplateObject &Element,Int32 *left_location=NULL,Int32 *right_location=NULL,Int32 (*compFunction)(TemplateObject &element1,TemplateObject &element2,void *context)=NULL,void *context=NULL);
			Int32						Find(TemplateObject &Element,Int32 start=POS_START,Int32 direction=DIR_FORWARD,Int32 (*compFunction)(TemplateObject &element1,TemplateObject &element2,void *context)=NULL,void *context=NULL);
			bool						Contains(TemplateObject &Element,Int32 (*compFunction)(TemplateObject &element1,TemplateObject &element2,void *context)=NULL,void *context=NULL);
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
			UInt32								start,end;

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
		template <class TemplateObject> Int32  GTVector<TemplateObject>::__Compare(TemplateObject &Element1,TemplateObject &Element2,Int32 (*compFunction)(TemplateObject &element1,TemplateObject &element2,void *context),void *context)
		{
			if (compFunction!=NULL)
			{
				return compFunction(Element1,Element2,context);
			} else {
				if (Element1<Element2)
					return -1;
				if (Element1>Element2)
					return 1;
				return 0;
			}	
		}		
		template <class TemplateObject> void   GTVector<TemplateObject>::__QSort(PTemplateElement<TemplateObject> *pData,Int32 (*compFunction)(TemplateObject &element1,TemplateObject &element2,void *context),Int32 lo,Int32 hi,bool ascendent,void *context)
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
					while ((lo<=right) && (__Compare(*(pData[lo].Element),*pivot->Element,compFunction,context)<=0))
						++lo;
					while ((hi>=left) &&  (__Compare(*(pData[hi].Element),*pivot->Element,compFunction,context)>0))
						--hi;
				} else {
					while ((lo<=right) && (__Compare(*(pData[lo].Element),*pivot->Element,compFunction,context)>=0))
						++lo;
					while ((hi>=left) &&  (__Compare(*(pData[hi].Element),*pivot->Element,compFunction,context)<0))
						--hi;
				}
				if (lo<hi)
				{
					GTVECTOR_SWAP(pData[lo],pData[hi]);

				}
			} 
			GTVECTOR_SWAP(pData[left],pData[hi]);

			// apelurile recursive
			__QSort(pData,compFunction,left,hi-1,ascendent,context);
			__QSort(pData,compFunction,hi+1,right,ascendent,context);
		}
		template <class TemplateObject> Int32  GTVector<TemplateObject>::__BinSearch(PTemplateElement<TemplateObject> *Data,TemplateObject& Element,Int32 (*compFunction)(TemplateObject &element1,TemplateObject &element2,void *context),Int32 lo,Int32 hi,void *context)
		{
			Int32 mij;
			Int32 res;

			do
			{
				mij=(lo+hi)/2;
				if ((res=__Compare(Element,*Data[mij].Element,compFunction,context))==0) 
					return mij;
				if (res<0) hi=mij-1;
				if (res>0) lo=mij+1;
			} while ((lo<=hi) && (hi>=0));
			return -1;
		}
		template <class TemplateObject> void   GTVector<TemplateObject>::Sort(bool ascendent,Int32 (*compFunction)(TemplateObject &element1,TemplateObject &element2,void *context),void *context)
		{
			if ((Data==NULL) || (ElementsCount==0)) 
				return;
			if (compFunction)
				__QSort((PTemplateElement<TemplateObject> *)Data,compFunction,0,ElementsCount-1,ascendent,context);
			else
				__QSort((PTemplateElement<TemplateObject> *)Data,compFnc,0,ElementsCount-1,ascendent,context);
		}
		template <class TemplateObject> bool   GTVector<TemplateObject>::BinarySearch(TemplateObject &Element, Int32 *left_location, Int32 *right_location,Int32 (*compFunctionToUse)(TemplateObject &element1,TemplateObject &element2,void *context),void *context)
		{
			Int32								poz;
			PTemplateElement<TemplateObject>	*ptrData;
			Int32 (*myCompFunction)(TemplateObject &element1,TemplateObject &element2,void *context) = NULL;

			if (compFunctionToUse!=NULL)
				myCompFunction = compFunctionToUse;
			else
				myCompFunction = compFnc;

			ptrData = (PTemplateElement<TemplateObject> *)Data;

			if ((Data==NULL) || (ElementsCount==0))
				return false;
			if ((poz=__BinSearch(ptrData,Element,myCompFunction,0,ElementsCount-1,context))==-1)
				return false;
			// merg in stanga daca e nevoie
			if (left_location!=NULL)
			{
				*left_location = poz-1;
				while (((*left_location)>=0) && (__Compare(*(ptrData[(*left_location)].Element),Element,myCompFunction,context)==0))
					(*left_location)--;
				(*left_location)++;
			}
			// merg in dreapta daca e nevoie
			if (right_location!=NULL)
			{
				*right_location = poz+1;
				while (((*right_location)<(Int32)ElementsCount) && (__Compare(*(ptrData[(*right_location)].Element),Element,myCompFunction,context)==0))
					(*right_location)++;
				(*right_location)--;
			}
			return true;
		}
		template <class TemplateObject> Int32  GTVector<TemplateObject>::Find(TemplateObject &Element, Int32 start, Int32 direction,Int32 (*compFunctionToUse)(TemplateObject &element1,TemplateObject &element2,void *context),void *context)
		{
			PTemplateElement<TemplateObject>	*ptrData;
			Int32 (*myCompFunction)(TemplateObject &element1,TemplateObject &element2,void *context) = NULL;

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
					if (__Compare(*(ptrData[start].Element),Element,myCompFunction,context)==0) 
						return start;
					start++;
				}
			} 
			if (direction==DIR_BACKWARD)
			{
				if ((start<0) || (start>=ElementsCount)) start=ElementsCount-1;
				while ((start>=0))
				{			
					if (__Compare(*(ptrData[start].Element),Element,myCompFunction,context)==0) 
						return start;
					start--;
				}
			}
			return -1;
		}
		template <class TemplateObject> bool   GTVector<TemplateObject>::Contains(TemplateObject &Element,Int32 (*compFunctionToUse)(TemplateObject &element1,TemplateObject &element2,void *context),void *context)
		{
			return (Find(Element,POS_START,DIR_FORWARD,compFunctionToUse,context)>=0);
		}
	}
}
#endif
//===================== Vector.h =================================================================================


typedef int					(*_BinarySearchCompFunction)(void *element1,void *element2);

namespace GML
{
	namespace Utils
	{
		class  Vector
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
			bool			Insert(void *Element,_BinarySearchCompFunction cmpFunc,bool ascendent);
			bool			CopyElement(unsigned int index,void *addr);
			bool			Delete(unsigned int index);
			bool			DeleteAll();
			bool			Resize(unsigned int newSize);
	
		};
	}
}
//===================== BitSet.h =================================================================================
#ifndef __BIT_SET__
#define __BIT_SET__


namespace GML
{
	namespace Utils
	{
		class  BitSet
		{
			UInt8	*Data;
			UInt32	Allocated;
			UInt32	ElementsCount;
		public:
			BitSet();
			~BitSet();

			bool	Create(UInt32 elements);
			void	Destroy();
			bool	Get(UInt32 poz);
			bool	Set(UInt32 poz,bool value);
			void	SetAll(bool value);
			bool	Reverse(UInt32 poz);
			void	ReverseAll();
			UInt32	Len();
			UInt32	GetAllocated();
			UInt8*	GetData();
			UInt32	CountElements(bool value);
		};
	}
}
#endif

//===================== Matrix.h =================================================================================

#define DIM_MAX 1000


namespace GML{
	namespace Utils{
			template <class T>
		class  Matrix{
		private:
			//the element "holder"
			GML::Utils::Vector	  v;
			//and the sizes of matrix
			unsigned int	nr_lines,
							nr_colls;
			//current allocated space
			unsigned int	max_lines,
							max_colls;
		public:
			//cosntructor - empty matrix
			Matrix(){
				nr_lines  = 0;
				nr_colls  = 0;
				//current allocated space
				max_lines = DIM_MAX;
				max_colls = DIM_MAX;
			}
			Matrix(Matrix<T>& m){
				this->max_colls = m.max_colls;
				this->max_lines = m.max_lines;
				this->nr_colls = m.nr_colls;
				this->nr_lines = m.nr_lines;
				this->v = m.v;
			}
			~Matrix(){
				nr_lines = 0;
				nr_colls = 0;
				max_lines = 0;
				max_colls = 0;
				v.Free();
			}

			//the way to alloc some space
			bool Create(unsigned int nrLines=10U,unsigned int nrColl=10U){
				//setting the allocated space of the matrix as a multiply of DIM_MAX
				if(nrColl > this->max_colls){
					this->max_colls = DIM_MAX*((unsigned int)nrColl/DIM_MAX + 1);
				}
				if(nrLines > this->max_lines){
					this->max_lines = DIM_MAX*((unsigned int)nrLines/DIM_MAX + 1);
				}
				//Allocate the space
				if(v.Create(max_lines * max_colls,sizeof(T)) == true){
					this->nr_colls = nrColl;
					this->nr_lines = nrLines;
					return true;
				}
				//if failure in constructing the vector
				this->max_lines = 0;
				this->max_lines = 0;
				this->nr_colls = 0;
				this->nr_lines = 0;
				return false;
			}
			
			// set a certain element. returns false if error
			bool SetElement(unsigned int line,unsigned int coll,T element){
				if(line >= this->max_lines || coll >= this->max_colls){
					return false;
				}
				((T*)v.GetVector())[line*this->max_colls + coll] = element;
				return true;
			}
			//gets a certain element
			T GetElement(unsigned int line,unsigned int coll){
				if(line >= this->max_colls || coll >= this->max_colls){
					return 0;
				}
				return ((T*)v.GetVector())[line*this->max_colls + coll]
			}
			/*
			bool AddElement(unsigned int line,unsigned int coll, T elem){
				if(line >= this->nr_lines || coll >= this->nr_colls){
					return false;
				}
				((T*)v.GetVector())[line*this->max_colls + coll] += element;
				return true;
			}
			*/
			unsigned int GetNrColl(){
				return this->nr_colls;
			}
			unsigned int GetNrLines(){
				return this->nr_lines;
			}
			unsigned int GetElementSize(){
				return this->v.GetElementSize();
			}
			unsigned int GetSize(){
				return this->max_colls*max_lines;
			}


			//Add operator. 
			Matrix<T> operator+(Matrix<T>& matrixRight){
				Matrix<T> m;
				unsigned int i,j;
				if(this->GetSize() == 0|| this->GetElementSize() != matrixRight.GetElementSize() || this->nr_colls != matrixRight.nr_colls || this->nr_lines != matrixRight.nr_lines){
					//is has 0 size
					return m;
				}
				m.Create(this->nr_lines,this->nr_colls);
				for(i=0;i<this->nr_lines;i++)
					for(j=0 ; j<this->nr_colls; j++)
						((T*)m.v.GetVector())[i*this->max_colls + j] = ((T*)this->v.GetVector())[i*this->max_colls + j] + ((T*)matrixRight.v.GetVector())[i*this->max_colls + j];
				for(i =this->nr_lines;i<this->max_lines;i++)
					m.ResetLine(i);
				for(i =this->nr_colls;i<this->max_colls;i++)
					m.ResetCollumn(i);
				return m;
			}

			//Multiply operator
			Matrix<T> operator*(Matrix<T>& matrixRight){
				Matrix<T> m;
				if(this->nr_colls != matrixRight.nr_lines || this->GetElementSize() != matrixRight.GetElementSize() || this->GetSize() == 0 || matrixRight.GetSize() == 0){
					//is has 0 size
					return m;
				}

				unsigned int r1 = this->nr_lines;
				unsigned int c2 = matrixRight.nr_colls;
				m.Create(r1,c2);
				T *v1 = (T*)this->v.GetVector(), *v2 = (T*)matrixRight.v.GetVector(), *v = (T*)m.v.GetVector();
				for(unsigned int i=0;i<this->nr_lines;i++){
					for(unsigned int j=0; j<matrixRight.nr_colls; j++){
						v[i * m.max_colls + j] = 0;
						for(unsigned int k=0; k<this->nr_colls; k++){
							v[i*m.max_colls+j] += (v1[i*this->max_colls + k] * v2[k*matrixRight->max_colls + j]);
						}
					}
				}
				return m;
			}

			//Grow the matrix
			bool Resize(unsigned int nrLines = 1U,unsigned int nrColls = 1U){
				unsigned int tempC = this->max_colls;
				unsigned int tempL = this->max_lines;
				bool realloc = false;
				unsigned int i,j;

				//if there must be more than max lines already allocated space
				if(this->nr_lines + nrLines > this->max_lines){
					tempL = DIM_MAX*((unsigned int)(nrLines + this->nr_lines)/DIM_MAX + 1);
					realloc = true;
				}
				//if there must be more than max colls already allocated space
				if(this->nr_colls + nrColls > this->max_colls){
					tempC = DIM_MAX*((unsigned int)(this->nr_colls + nrColls)/DIM_MAX + 1);
					realloc = true;
				}
				//we resize the support vector
				if(realloc){
					if(this->v.Resize(tempC*tempL) ==false){
						return false;
					}
					//update the allocated dimensions to the real ones
					T* v = (T*)this->v.GetVector();

					//Reorder the data inside the vector
					for(i = this->nr_lines-1;i>0;i--){
						for(j = this->nr_colls-1;j!=0xffffffff;j--){
							v[i*tempC + j] = v[i*this->max_colls + j];
						}
					}

					this->max_lines = tempL;
					this->max_colls = tempC;

					//we set to zero the lines allocated, but not yet popullated
					for(i = this->nr_lines;i<this->max_lines;i++)
						ResetLine(i);
					for(i = this->nr_colls;i<this->max_colls;i++)
						ResetCollumn(i);
				}
				
				//and after that sets the correct number of colls/lines
				this->nr_colls += nrColls;
				this->nr_lines += nrLines;
				
				return true;
			}
			//sets to zero a certain line
			bool ResetLine(unsigned int line){
				if(line >= this->max_lines){
					return false;
				}
				
				for(unsigned int coll=0;coll<this->max_colls;coll++){
					this->SetElement(line,coll,0);
				}
				return true;
			}
			//sets to zero a certain column
			bool ResetCollumn(unsigned int coll){
				if(coll >= this->max_colls){
					return false;
				}
				
				for(unsigned int line=0;line<this->max_lines;line++){
					this->SetElement(line,coll,0);
				}
				return true;
			}

			//resets the matrix
			void Reset(){
				for(unsigned int i=0;i<this->max_lines;i++){
					for(unsigned int j=0; j<this->max_colls; j++){
						
						((T*)v.GetVector())[i * this->max_colls + j] = 0;
					}
				}
			}
		};		
	}
}
//===================== FixMatrix.h =================================================================================
#ifndef __FIXMATRIX_H__
#define __FIXMATRIX_H__


namespace GML{
	namespace Utils{
			template <class T>
		class  FixMatrix{
		private:
			T* data;
			unsigned int	lines,
							collumns;
		public:
			FixMatrix(unsigned int lines,unsigned int collumns){
				if((data = new T[lines*collumns*sizeof(T)] )== NULL){
					data = NULL;
					this->lines = this->collumns = 0;
				}else{
					this->lines = lines;
					this->collumns = collumns;
				}
			}
			FixMatrix(){data = NULL;lines=collumns=0;}
			~FixMatrix(){
				Free();
			}

			bool Create(unsigned int lines,unsigned int collumns){
				T* temp;
				if(this->lines == lines && this->collumns == collumns){
					return true;
				}
				if((temp = new T[lines*collumns*sizeof(T)] )== NULL){
					return false;
				}
				data = temp;
				this->lines		= lines;
				this->collumns  = collumns;
				return true;
			}
			//It works directly on the result object so you have to take care
			bool Multiply(FixMatrix<T>& m,FixMatrix<T>& result){
				if(this->collumns != m.lines){
					//is has 0 size
					return false;
				}

				unsigned int r1 = this->lines;
				unsigned int c2 = m.collumns;
				
				if(result.Create(this->lines,m.collumns) == false){
					return false;
				}

				for(unsigned int i=0;i<this->lines;i++){
					for(unsigned int j=0; j<m.collumns; j++){
						result.data[i * result.collumns + j] = 0;
						for(unsigned int k=0; k < this->collumns; k++){
							result.data[i*result.collumns+j] += (this->data[i*this->collumns + k] * m.data[k*m.collumns + j]);
						}
					}
				}
				return true;
			}
			//It works directly on the result object so you have to take care
			bool Add(FixMatrix<T>&m, FixMatrix<T>& result){
				unsigned int i,j;
				if(this->collumns!= m.collumns|| this->lines != m.lines){
					//is has 0 size
					return false;
				}
				if(result.Create(this->lines,this->collumns) == false){
					return false;
				}
				for(i=0;i<this->lines;i++)
					for(j=0 ; j<this->collumns; j++)
						m.data[i*m.collumns+ j] = this->data[i*this->collumns + j] + m.data[i*m.collumns+ j];
				return true;
			}

			T* GetValue(unsigned int line,unsigned int collumn){
				if(line >= this->lines || collumn >= this->collumns){
					return NULL;
				}
				return &this->data[line*this->collumns + collumn];
			}

			bool setValue(unsigned int line,unsigned int collumn, T val){
				if(line >= this->lines || collumn >= this->collumns){
					return false;
				}
				this->data[line*this->collumns + collumn] = val;
				return true;
			}

			void Free(){
				if (data!=NULL) delete data;
				lines = collumns = 0;	
				data=NULL;
			}
		};
	}
}
#endif
//===================== GTFVector.h =================================================================================
#ifndef __GTF_VECTOR__
#define __GTF_VECTOR__


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
			GTFVector(UInt32 initialSize);

			bool						Create(UInt32 initialSize);
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
	}
}

#endif

//===================== Indexes.h =================================================================================
#ifndef __INDEXES__
#define __INDEXES__


#define INVALID_INDEX	((UInt32)(-1))

namespace GML
{
	namespace Utils
	{
		class  Interval
		{
		public:
			UInt32	Start,End;

			Interval();
			void	Set(UInt32 _start,UInt32 _end);
			UInt32	Size();
		};
		class  Indexes
		{
		protected:
			UInt32		*List;
			UInt32		MaxAlloc;
			UInt32		Count;
		public:
			Indexes();
			~Indexes();
			
			bool		Create(UInt32 count);
			void		Destroy();
			
			bool		Push(UInt32 index);
			UInt32		Get(UInt32 poz);
			UInt32*		GetList();			
			UInt32		Len();
			UInt32		GetTotalAlloc();
			bool		Truncate(UInt32 newCount);	
			bool		CopyTo(GML::Utils::Indexes &idx);
		};
	}
}

#endif


//===================== md5.h =================================================================================


namespace GML
{
	namespace Utils
	{ 
		class  MD5
		{
			struct MD5Context 
			{
				unsigned int	buf[4];
				unsigned int	bits[2];
				unsigned char	in[64]; 
				unsigned char	digest[16];
				char			stringResult[33];
			};

			MD5Context		context;
		public:
			void			Init();
			void			Update(void *Buffer,unsigned int size);
			void			Final();
			char*			GetResult();		
		};
	}
}


//===================== GString.h =================================================================================



#define TCHAR				char
#define TYPE_UINT64			unsigned __int64
#define TYPE_INT64			__int64	

class File;
namespace GML
{
	namespace Utils
	{
		class  GString
		{
			TCHAR	*Text;
			int		Size;
			int		MaxSize;

		public:
			bool	Grow(int newSize);
		public:
			GString(void);
			~GString(void);

		// functii statice
			static int		Len(TCHAR *string);
			static bool		Add(TCHAR *destination,TCHAR *text,int maxDestinationSize,int destinationTextSize = -1,int textSize = -1);	
			static bool		Set(TCHAR *destination,TCHAR *text,int maxDestinationSize,int textSize = -1);
			static bool		Equals(TCHAR *sir1,TCHAR *sir2,bool ignoreCase=false);
			static int		Compare(TCHAR *sir1,TCHAR *sir2,bool ignoreCase=false);
			static bool		StartsWith(TCHAR *sir,TCHAR *text,bool ignoreCase=false);					
			static bool		EndsWith(TCHAR *sir,TCHAR *text,bool ignoreCase=false,int sirTextSize = -1,int textSize = -1);
			static int		Find(TCHAR *source,TCHAR *text,bool ignoreCase=false,int startPoz=0,bool searchForward=true,int sourceSize = -1);
			static bool		Delete(TCHAR *source,int start,int end,int sourceSize=-1);
			static bool		Insert(TCHAR *source,TCHAR *text,int pos,int maxSourceSize,int sourceSize=-1,int textSize=-1);
			static bool		ReplaceOnPos(TCHAR *source,TCHAR *text,int start,int end,int maxSourceSize,int sourceSize=-1,int textSize=-1);
			static bool		Replace(TCHAR *source,TCHAR *pattern,TCHAR *newText,int maxSourceSize,bool ignoreCase=false,int sourceSize=-1,int patternSize=-1,int newTextSize=-1);
			static bool		ConvertToUInt8(TCHAR *text,unsigned char *value,unsigned int base=0,int textSize=-1);
			static bool		ConvertToUInt16(TCHAR *text,unsigned short *value,unsigned int base=0,int textSize=-1);
			static bool		ConvertToUInt32(TCHAR *text,unsigned int *value,unsigned int base=0,int textSize=-1);
			static bool		ConvertToUInt64(TCHAR *text,TYPE_UINT64 *value,unsigned int base=0,int textSize=-1);
			static bool		ConvertToInt8(TCHAR *text,char *value,unsigned int base=0,int textSize=-1);
			static bool		ConvertToInt16(TCHAR *text,short *value,unsigned int base=0,int textSize=-1);
			static bool		ConvertToInt32(TCHAR *text,int *value,unsigned int base=0,int textSize=-1);
			static bool		ConvertToInt64(TCHAR *text,TYPE_INT64 *value,unsigned int base=0,int textSize=-1);
			static bool		ConvertToDouble(TCHAR *text,double *value,int textSize=-1);
			static bool		ConvertToFloat(TCHAR *text,float *value,int textSize=-1);

		// functii obiect
			operator		TCHAR *() { return Text; }

			char			*GetText() { return Text; }
			int				GetSize() { return Size; }
			int				Len() { return Size; }

			bool			Add(TCHAR *ss,int size=-1);
			bool			AddChar(char ch);
			bool			Add(GString *ss,int size=-1);

			bool			Set(TCHAR *ss,int size=-1);
			bool			Set(GString *ss,int size=-1);

			bool			SetFormated(TCHAR *format,...);
			bool			AddFormated(TCHAR *format,...);

			bool			Create(int initialSize=64);
			bool			LoadFromFile(TCHAR *fileName,int start=0,int end=-1);
			void			Distroy();
			void			Truncate(int newSize);

			int				Compare(TCHAR *ss,bool ignoreCase=false);
			int				Compare(GString *ss,bool ignoreCase=false);

			bool			StartsWith(TCHAR *ss,bool ignoreCase=false);
			bool			StartsWith(GString *ss,bool ignoreCase=false);
			bool			EndsWith(TCHAR *ss,bool ignoreCase=false);
			bool			EndsWith(GString *ss,bool ignoreCase=false);
			bool			Contains(TCHAR *ss,bool ignoreCase=false);
			bool			Contains(GString *ss,bool ignoreCase=false);
			bool			Equals(TCHAR *ss,bool ignoreCase=false);
			bool			Equals(GString *ss,bool ignoreCase=false);

			int				Find(TCHAR *ss,bool ignoreCase=false,int startPoz=0,bool searchForward=true);
			int				Find(GString *ss,bool ignoreCase=false,int startPoz=0,bool searchForward=true);
			int				FindLast(TCHAR *ss,bool ignoreCase=false);
			int				FindLast(GString *ss,bool ignoreCase=false);

			bool			Delete(int start,int end);
			bool			DeleteChar(int pos);
			bool			Insert(TCHAR *ss,int pos);
			bool			InsertChar(TCHAR ch,int pos);
			bool			Insert(GString *ss,int pos);
			bool			ReplaceOnPos(int start,int end,TCHAR *newText,int newTextSize=-1);
			bool			ReplaceOnPos(int start,int end,GString *ss);
			bool			Replace(TCHAR *pattern,TCHAR *newText,bool ignoreCase=false,int patternSize = -1,int newTextSize = -1);
			bool			Replace(GString *pattern,GString *newText,bool ignoreCase=false);
			bool			Strip(TCHAR *charList=NULL,bool stripFromLeft=true,bool stripFromRight=true);

			bool			Split(TCHAR *separator,GString *arrayList,int arrayListCount,int *elements=NULL,int separatorSize=-1);
			bool			Split(GString *separator,GString *arrayList,int arrayListCount,int *elements=NULL);

			bool			CopyNextLine(GString *line,int *position);
			bool			CopyNext(GString *token,TCHAR *separator,int *position,bool ignoreCase=false);
		
			bool			CopyPathName(GString *path);
			bool			CopyFileName(GString *path);
			bool			PathJoinName(TCHAR *name,TCHAR separator = '\\');
			bool			PathJoinName(GString *name,TCHAR separator = '\\');

			bool			MatchSimple(TCHAR *mask,bool ignoreCase=false);
			bool			MatchSimple(GString *mask,bool ignoreCase=false);

			bool			ConvertToInt8(char *value,unsigned int base=0);
			bool			ConvertToInt16(short *value,unsigned int base=0);
			bool			ConvertToInt32(int *value,unsigned int base=0);
			bool			ConvertToInt64(TYPE_INT64 *value,unsigned int base=0);

			bool			ConvertToUInt8(unsigned char *value,unsigned int base=0);
			bool			ConvertToUInt16(unsigned short *value,unsigned int base=0);
			bool			ConvertToUInt32(unsigned int *value,unsigned int base=0);
			bool			ConvertToUInt64(TYPE_UINT64 *value,unsigned int base=0);

			bool			ConvertToDouble(double *value);
			bool			ConvertToFloat(float *value);

			void			NumberToString(TYPE_UINT64 number,unsigned int base,int max_size=-1,int fill=-1);


		};
	}
}

//===================== File.h =================================================================================

class GString;
namespace GML
{
	namespace Utils
	{
		class  File
		{
			FILE_HANDLE		hFile;
		public:
			File(void);
			~File(void);

			bool			Create(char *name,bool share=false);			
			bool			OpenRead(char *name,bool share=false);
			bool			OpenReadWrite(char *name,bool append=false,bool share=false);
			void			Close();
			UInt32			GetFileSize();
			UInt32			GetFilePos();
			bool			SetFilePos(UInt32 pos);
			bool			Read(void *Buffer,UInt32 size,UInt32 *readSize=NULL);
			bool			Write(void *Buffer,UInt32 size,UInt32 *writeSize=NULL);
			bool			ReadNextLine(GString &line,bool skipEmpyLines=true);
			bool			Flush();
		};
	}
}

//===================== Timer.h =================================================================================


namespace GML
{
	namespace Utils
	{
		class  Timer
		{
			UInt32			timeStart,timeDiff;			
		public:

			void			Start();
			void			Stop();
			UInt32			GetPeriodAsMiliSeconds();
			char*			GetPeriodAsString(GString &str);
			char*			EstimateTotalTime(GString &str,UInt32 parts,UInt32 total);
			char*			EstimateETA(GString &str,UInt32 parts,UInt32 total);
		};
	}
}

//===================== AlgorithmResult.h =================================================================================


namespace GML
{
	namespace Utils
	{
		class  AlgorithmResult
		{
		public:
			UInt32		Iteration;
			double		tp,tn,fp,fn;
			double		sp,se,acc;
			Timer		time;

		public:
			AlgorithmResult(void);

			void		Update(bool classType,bool corectellyClasified,double updateValue = 1);
			void		Clear();
			void		Copy(AlgorithmResult *res);
			void		Add(AlgorithmResult *res);
			void		Compute();
		};
	}
}

//===================== IParalelUnit.h =================================================================================


namespace GML
{
	namespace Utils
	{
		class  IParalelUnit
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

//===================== ThreadParalelUnit.h =================================================================================

namespace GML
{
	namespace Utils
	{
		class  ThreadParalelUnit : public IParalelUnit
		{
			HANDLE			eventWorking,eventWaiting,hThread;
		public:
			ThreadParalelUnit(void);
			~ThreadParalelUnit(void);

			void			DoAction();
			bool			WaitToFinish();
			bool			Execute(unsigned int codeID);
		};
	}
};

//===================== AttributeList.h =================================================================================




namespace GML
{
	namespace Utils
	{
		enum AttributeFlags
		{
			FL_LIST = 1,
			FL_FILEPATH = 2,
		};
		struct  Attribute
		{
			char*			Name;
			char*			MetaData;
			unsigned char*	Data;
			unsigned int	DataSize;
			unsigned int	AttributeType;
			unsigned int	ElementsCount;

			bool operator < (Attribute &a1);
			bool operator > (Attribute &a1);

			bool			GetListItem(GML::Utils::GString &str);
			bool			GetDescription(GML::Utils::GString &str);
			UInt32			GetFlags();
		};

		struct  AttributeLink
		{
			char*			Name;
			void*			LocalAddress;
			unsigned int	AttributeType;
			char*			MetaData;
		};

		class  AttributeList
		{
			GTVector<GML::Utils::Attribute>	list;
			
			bool			FromString(GML::Utils::GString &text);
		public:
			enum 
			{
				BOOLEAN = 0,
				INT8,INT16,INT32,INT64,
				UINT8,UINT16,UINT32,UINT64,
				FLOAT,DOUBLE,
				STRING,
				ATTRIBUTES_COUNT		
			};		
		public:
			AttributeList(void);
			~AttributeList(void);
		
			bool			AddAttribute(char* Name,void *Data,unsigned int AttributeType,unsigned int ElementsCount=1,char *Description=NULL);

			bool			AddString(char *Name, char *Text, char *Description = NULL);
			bool			AddBool(char *Name, bool value,char *Description = NULL);
			bool			AddDouble(char *Name, double value, char *Description = NULL);
			bool			AddUInt32(char *Name, UInt32 value, char *Description = NULL);
			bool			AddInt32(char *Name, Int32 value, char *Description = NULL);

			bool			Update(char *Name,void *Data,UInt32 DataSize);

			bool			UpdateBool(char *Name,bool &boolValue,bool useDefault=false,bool defaultValue=false);
			bool			UpdateUInt32(char *Name,UInt32 &uint32Value,bool useDefault=false,UInt32 defaultValue=0);
			bool			UpdateInt32(char *Name,Int32 &int32Value,bool useDefault=false,Int32 defaultValue=0);
			bool			UpdateDouble(char *Name,double &doubleValue,bool useDefault=false,double defaultValue=0);
			bool			UpdateString(char *Name,GML::Utils::GString &text,bool useDefault=false,char* defaultValue="");
			
			void			Clear();
			Attribute*		Get(unsigned int index);
			Attribute*		Get(char* Name);
			unsigned int	GetCount();

			bool			Save(char *fileName);
			bool			Load(char *fileName);
			bool			Create(char *text,char separator=';');			
		};
	}
}
//===================== GMLObject.h =================================================================================
#ifndef __GML_OBJECT__
#define __GML_OBJECT__


namespace GML
{
	namespace Utils
	{
		class  GMLObject
		{
			GML::Utils::GTFVector<GML::Utils::AttributeLink>	AttrLinks;
		protected:
			char*												ObjectName;
		protected:
			bool						LinkPropertyToString(char *Name,GML::Utils::GString &LocalAddr,char *defaultValue,char *MetaData=NULL);
			bool						LinkPropertyToBool(char *Name,bool &LocalAddr,bool defaultValue,char *MetaData=NULL);
			bool						LinkPropertyToDouble(char *Name,double &LocalAddr,double defaultValue,char *MetaData=NULL);
			bool						LinkPropertyToUInt32(char *Name,UInt32 &LocalAddr,UInt32 defaultValue,char *MetaData=NULL);
			bool						LinkPropertyToInt32(char *Name,Int32 &LocalAddr,Int32 defaultValue,char *MetaData=NULL);
			bool						RemoveProperty(char *Name);
			bool						SetPropertyMetaData(char *Name,char *MetaData);
			
		
		public:
			bool						SetProperty(GML::Utils::AttributeList &config);
			bool						SetProperty(char *config);
			bool						SetPropertyFromFile(char *fileName);
			
			bool						GetProperty(GML::Utils::AttributeList &config);
			bool						SavePropery(char *fileName);
			char*						GetObjectName();
		public:
			GMLObject();
		};
	}
}

#endif


//===================== INotifier.h =================================================================================


namespace GML
{
	namespace Utils
	{
		class  INotifier: public GMLObject
		{

		public:
			enum {
				NOTIFY_ERROR = 0,
				NOTIFY_INFO,
				NOTIFY_START_PROCENT,
				NOTIFY_END_PROCENT,
				NOTIFY_PROCENT,		
				NOTIFY_RESULT = 100,
			};
			bool			Init(char *attributeString);

			virtual bool	OnInit() = 0;
			virtual bool	Uninit() = 0;
			virtual bool	Notify(UInt32 messageID,void *Data,UInt32 DataSize) = 0;

			bool			NotifyString(UInt32 messageID,char* format,...);
			bool			Info(char *format,...);
			bool			Error(char *format,...);
			bool			StartProcent(char *format,...);
			bool			SetProcent(double procValue);
			bool			SetProcent(double procValue,double maxValue);
			bool			EndProcent();
			bool			Result(GML::Utils::AlgorithmResult &ar);
		};
	}
}
//===================== VectorOp.h =================================================================================
#ifndef __VECTOR__OP__
#define __VECTOR__OP__


#define Mean	Average

namespace GML
{
	namespace ML
	{
		class  VectorOp
		{
		public:
			static void		AddVectors(double *v1,double *v2,UInt32 elements);
			static double	ComputeVectorsSum(double *v1,double *v2,UInt32 elements);
			// perceptron specific
			static bool		IsPerceptronTrained(double *features,double *weights,UInt32 elements,double label);
			static bool		IsPerceptronTrained(double *features,double *weights,UInt32 elements,double b,double label);
			static void		AdjustTwoStatePerceptronWeights(double *features,double *weights,UInt32 elements,double error);
			static void		AdjustTwoStatePerceptronWeights(double *features,double *weights,UInt32 elements,double error,double *featuresWeight);			
			static void		AdjustPerceptronWeights(double *features,double *weights,UInt32 elements,double error);
			static void		AdjustPerceptronWeights(double *features,double *weights,UInt32 elements,double error,double *featuresWeight);
			static double	PointToPlaneDistance(double *plane,double *point,UInt32 elements,double planeBias);
			static double	PointToPlaneDistanceSquared(double *plane,double *point,UInt32 elements,double planeBias);
			static double	PointToPlaneDistanceSigned(double *plane,double *point,UInt32 elements,double planeBias);	
			static double	PointToPlaneDistanceSquaredSigned(double *plane,double *point,UInt32 elements,double planeBias);			
			static double	PointToPointDistanceSquared(double *p1,double *p2,UInt32 elements);
			static double	PointToPointDistance(double *p1,double *p2,UInt32 elements);
			static double	PointToPointDistanceSquared(double *p1,double *p2,double *pWeight,UInt32 elements);
			static double	PointToPointDistance(double *p1,double *p2,double *pWeight,UInt32 elements);
			static double	Average(double *v,UInt32 elements);
			static double	StandardDeviation(double *v,UInt32 elements);
			
		};
	}
}


#endif

//===================== DBRecord.h =================================================================================


namespace GML
{
	namespace DB
	{
		enum DBRecordDataType
		{
			NULLVAL = 0,
			BOOLVAL,
			UINT8VAL,
			UINT16VAL,
			UINT32VAL,
			UINT64VAL,
			INT8VAL,
			INT16VAL,
			INT32VAL,
			RAWPTRVAL,
			BYTESVAL,
			ASCIISTTVAL,
			UNICSTRVAL,
			DOUBLEVAL,
			FLOATVAL,
			HASHVAL
		};
		struct  RecordHash
		{
		public:
			union
			{
				UInt8		bValue[16];
				UInt32		dwValue[4];
			} Hash;
		public:
			bool	CreateFromText(char *text);
			bool	ToString(GML::Utils::GString &str);
			void	Copy(RecordHash &rHash);
			void	Reset();
		};
		struct  DBRecord 
		{
			UInt32			Type;
			char*			Name;
			UInt32			Size;
			union
			{
				bool		BoolVal;
				Int8		Int8Val;
				Int16		Int16Val;
				Int32		Int32Val;
				Int64		Int64Val;
				UInt8		UInt8Val;
				UInt16		UInt16Val;
				UInt32		UInt32Val;
				UInt64		UInt64Val;
				void*		RawPtrVal;
				UInt8*		BytesVal;
				double		DoubleVal;
				float		FloatVal;
				char*		AsciiStrVal;
				wchar_t*	UnicStrVal;
				RecordHash	Hash;
			};
		};
	}
}


//===================== MLRecord.h =================================================================================


namespace GML
{
	namespace ML
	{
		struct  MLRecord
		{
			UInt32				FeatCount;
			double*				Features;
			double				Weight;
			GML::DB::RecordHash	Hash;	
			double				Label;
			MLRecord*			Parent;
		};
	}
}


//===================== IDataBase.h =================================================================================



namespace GML
{
	namespace DB
	{
		class  IDataBase: public GML::Utils::GMLObject
		{
		protected:
			GML::Utils::INotifier			*notifier;

		public:
			virtual ~IDataBase();
			bool						Init (GML::Utils::INotifier &notifier, char *connectionString);
			virtual bool				OnInit()=0;			
			virtual bool				Connect ()=0;
			virtual bool				Disconnect ()=0;
			virtual bool				ExecuteQuery(char* Statement,UInt32 *rowsCount=NULL)=0;
			virtual bool				FetchNextRow (GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr)=0;
			virtual bool				GetColumnInformations (GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr)=0;

			 /*
			  *Usage: insert a new ENTIRE row into the database
			  *Param:
			  *	- INPUT char* Table: the table the data is to be inserted in
			  *	- INPUT DbRecordVect * Vect: a vector of Record objects to be inserted
			  *	Return: true/false if the action succeded or not
			  */
			virtual bool				InsertRow (char* Table, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect)=0;

			/*
			  *Usage: insert a new ENTIRE row into the database
			  *Param:
			  *	- INPUT char* Table: the table the data is to be inserted in
			  *	- INPUT char* Fields: a string of fields to be inserted separated by comma
			  *	- INPUT DbRecordVect * Vect: a vector of Record objects to be inserted
			  *	Return: true/false if the action succeded or not
			  */
			virtual bool				InsertRow (char* Table, char* Fields, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect)=0;

			/*
			 * Usage: execute a sql update statement 
			 * Param:
			 *  - INPUT char* SqlStatement: the update sql statement
			 *  - INPUT DbRecordVect* WhereVals: the values used to create the where part of the statement
			 *  - INPUT DbRecordVect* UpdateVals: the values used to replace the old values
			 * Return: true/false if the operation succeded or not
			 */
			virtual bool				Update (char* SqlStatement, GML::Utils::GTFVector<GML::DB::DBRecord> &WhereVals, GML::Utils::GTFVector<GML::DB::DBRecord> &UpdateVals)=0;
		};
	}
}



//===================== MLInterface.h =================================================================================



namespace GML
{
	namespace ML
	{
		enum RecordMask
		{
			RECORD_STORE_HASH = 1,
		};
		struct TableColumnIndexes
		{
			UInt32				nrFeatures;
			Int32				indexLabel;
			Int32				indexHash;
			Int32				*indexFeature;
			GML::Utils::GString	*featName;
		};
		class  IConnector : public GML::Utils::GMLObject
		{
		private:
			bool						Init(GML::Utils::INotifier &Notifier,GML::DB::IDataBase *Database,GML::ML::IConnector *connecor,char *attributeString);
		protected:
			GML::Utils::INotifier		*notifier;			
			GML::DB::IDataBase			*database;
			GML::ML::IConnector			*conector;
			GML::Utils::GString			DataFileName;			
			GML::Utils::GString			Query;
			GML::Utils::GString			CountQuery;
			TableColumnIndexes			columns;
			UInt32						CachedRecords;
			bool						StoreRecordHash;
			
			void						ClearColumnIndexes();
			bool						UpdateDoubleValue(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr,Int32 index,double &value);
			bool						UpdateHashValue(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr,Int32 index,GML::DB::RecordHash &recHash);
			bool						UpdateColumnInformations(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr);
			bool						QueryRecordsCount(char *CountQueryStatement,UInt32 &recordsCount);
			bool						UpdateColumnInformations(char *QueryStatement);


			virtual bool				OnInit();
			virtual bool				OnInitConnectionToDataBase();
			virtual bool				OnInitConnectionToConnector();
			virtual bool				OnInitConnectionToCache();
			
		public:	
			IConnector();

			virtual bool				Init(GML::Utils::INotifier &Notifier,GML::DB::IDataBase &Database,char *attributeString=NULL);
			virtual bool				Init(GML::ML::IConnector &conector,char *attributeString=NULL);
			virtual bool				Init(GML::Utils::INotifier &Notifier,char *attributeString=NULL);
			virtual bool				Save(char *fileName);
			virtual bool				Load(char *fileName);


			virtual bool				Close()=0;	
			virtual bool				CreateMlRecord (MLRecord &record)=0;
			virtual bool				FreeMLRecord(MLRecord &record)=0;

			virtual bool				GetRecord(MLRecord &record,UInt32 index,UInt32 recordMask=0)=0;
			virtual bool				GetRecordLabel(double &label,UInt32 index)=0;
			virtual bool				GetRecordHash(GML::DB::RecordHash &recHash,UInt32 index);
			virtual bool				GetFeatureName(GML::Utils::GString &str,UInt32 index);

			virtual UInt32				GetFeatureCount()=0;
			virtual UInt32				GetRecordCount()=0;	
			virtual UInt32				GetTotalRecordCount()=0;

			virtual bool				SetRecordInterval(UInt32 start, UInt32 end)=0;
		};

	}
}



//===================== IAlgorithm.h =================================================================================
#ifndef __I_ALGORITHM__
#define __I_ALGORITHM__




namespace GML
{
	namespace Algorithm
	{
		class  IAlgorithm: public GML::Utils::GMLObject
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

//===================== Builder.h =================================================================================



namespace GML
{
	class  Builder
	{
	public:
		static GML::Utils::INotifier*		CreateNotifier(char *pluginName);	
		static GML::DB::IDataBase*			CreateDataBase(char *pluginName,GML::Utils::INotifier &notify);
		static GML::ML::IConnector*			CreateConnectors(char *conectorsList,GML::Utils::INotifier &notify,GML::DB::IDataBase &database);
		static GML::ML::IConnector*			CreateConnectors(char *conectorsList,GML::Utils::INotifier &notify);
		static GML::Algorithm::IAlgorithm*	CreateAlgorithm(char *algorithmLib);	
		static bool							GetPluginProperties(char *pluginName,GML::Utils::AttributeList &attr);
	};
}


//===================== IMLAlgorithm.h =================================================================================
#ifndef __INTERFACE_ML_ALGORITHM__
#define __INTERFACE_ML_ALGORITHM__


namespace GML
{
	namespace Algorithm
	{
		struct  MLThreadData
		{
			UInt32							ThreadID;
			GML::ML::MLRecord				Record;
			GML::Utils::AlgorithmResult		Res;
			GML::Utils::Interval			Range;
			void*							Context;
		};
		class  IMLAlgorithm: public GML::Algorithm::IAlgorithm
		{
		public:
			MLThreadData					*ThData;

		protected:
			// properties
			UInt32							threadsCount;
			GML::Utils::GString				Conector;
			GML::Utils::GString				DataBase;
			GML::Utils::GString				Notifier;
			GML::DB::IDataBase				*db;
			GML::ML::IConnector				*con;

			// local variables
			GML::Utils::ThreadParalelUnit	*tpu;
			

			bool							InitConnections();
			bool							InitThreads();
			bool							ExecuteParalelCommand(UInt32 command);
			bool							SplitMLThreadDataRange(UInt32 maxCount);

			virtual bool					OnInitThreadData(GML::Algorithm::MLThreadData &thData);
			virtual bool					OnInitThreads();

		public:
			IMLAlgorithm();	
			virtual void					OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand);

		};
	}
}

#endif

#endif

