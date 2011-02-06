#ifndef __GML_LIB__
#define __GML_LIB__

#ifndef __COMPAT_H
#define __COMPAT_H

#define NOTIFYER_EXT	".ntf"
#define DATABASE_EXT	".db"
#define CONNECTOR_EXT	".dbc"
#define ALGORITHM_EXT	".alg"

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
//===================== File.h =================================================================================

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

			bool			Create(char *name);
			bool			OpenRead(char *name);
			bool			OpenReadWrite(char *name,bool append=false);
			void			Close();
			UInt32			GetFileSize();
			UInt32			GetFilePos();
			bool			SetFilePos(UInt32 pos);
			bool			Read(void *Buffer,UInt32 size,UInt32 *readSize=NULL);
			bool			Write(void *Buffer,UInt32 size,UInt32 *writeSize=NULL);
		};
	}
}

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

//===================== Timer.h =================================================================================


namespace GML
{
	namespace Utils
	{
		class  Timer
		{
			UInt32			timeStart,timeDiff;
			GString			Result;
		public:

			void			Start();
			void			Stop();
			UInt32			GetPeriodAsMiliSeconds();
			char*			GetPeriodAsString();
			char*			EstimateTotalTime(UInt32 parts,UInt32 total);
			char*			EstimateETA(UInt32 parts,UInt32 total);

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

		public:
			AlgorithmResult(void);

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

//===================== INotify.h =================================================================================


namespace GML
{
	namespace Utils
	{
		class  INotify
		{
		public:
			enum {
				NOTIFY_ERROR = 0,
				NOTIFY_INFO,
			};
			virtual bool	Init(void *initData) = 0;
			virtual bool	Uninit() = 0;
			virtual bool	Notify(UInt32 messageID,void *Data,UInt32 DataSize) = 0;

			bool			NotifyString(UInt32 messageID,char* format,...);
			bool			Info(char *format,...);
			bool			Error(char *format,...);
		};
	}
}
//===================== DBRecord.h =================================================================================


namespace GML
{
	namespace DB
	{
		enum DBRecordDataType
		{
			NULLVAL = 0,
			UINT8VAL,
			UINT16VAL,
			UINT32VAL,
			UINT64VAL,
			RAWPTRVAL,
			BYTESVAL,
			ASCIISTTVAL,
			UNICSTRVAL,
			HASHVAL
		};
		struct  RecordHash
		{
			UInt8		Value[16];
		};
		struct  DBRecord 
		{
			UInt32			Type;
			char*			Name;
			union
			{
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
		};
	}
}


//===================== AttributeList.h =================================================================================


namespace GML
{
	namespace Utils
	{
		struct  Attribute
		{
			char*			Name;
			char*			Description;
			unsigned char*	Data;
			unsigned int	DataSize;
			unsigned int	AttributeType;
			unsigned int	ElementsCount;

			bool operator < (Attribute &a1);
			bool operator > (Attribute &a1);
		};

		class  AttributeList
		{
			GML::Utils::GTVector<GML::Utils::Attribute>	list;
			
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
			bool			UpdateString(char *Name,GML::Utils::GString &text);

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
//===================== IDataBase.h =================================================================================



namespace GML
{
	namespace DB
	{
		class  IDataBase 
		{
		protected:
			/*
			 * Generic Notifier object for passing messages
			 *  - in the case of this class mostly errors 
			 */
			GML::Utils::INotify			*notifier;
			GML::Utils::AttributeList	Attr;

		public:
			/*
			 * Cand se apeleaza OnInit() , notifier-ul este deja setat iar in Attr sunt incarcate toate atributele
			 * din conectionString 
			 */
			virtual bool				OnInit() = 0;

			bool						Init (GML::Utils::INotify &notifier, char *connectionString);

			/*
			 * Usage: 
			 * - connect to the desired database	 
			 * Param:
			 *	- INPUT char* Database: the database name to connect to
			 *	- INPUT OPT char* Username: the username credential
			 *	- INPUT OPT char* Password: the password credential
			 *	- INPUT OPT UInt Port: an optional parameter that specified the port 
			 *	Return: true/false if we have a connection or not
			 */
			virtual bool				Connect ()=0;

			/*
			 * Usage: Disconnect from the database
			 * Return: true/false if the operation succeded or not
			 */
			virtual bool				Disconnect ()=0;

			/*
			 * Usage: emit a sql select statement to fetch new data
			 * Param: 
			 *	- INPUT char* SqlStatement: the sql select statement
			 * Return: the number of records fetched during the statement execution
			 */
			virtual UInt32				Select (char* Statement="*")=0;

			/*
			 * Usage: emit a sql select statement that is broken in 3 pieces
			 * Param: 
			 *	- INPUT char* What:  what columns to select (it can be "*" for all of them)
			 *  - INPUT char* Where: the sql conditions for the select
			 *  - INPUT char* From:  what table to select from
			 *  Return: the number of rows fetched during statement executution
			 */
			virtual UInt32				SqlSelect (char* What="*", char* Where="", char* From="")=0;
	
			/*
			 * Usage: fetch a new record after a previous SqlSelect call
			 * Param:
			 *	- INPUT/OUTPUT DbRecordVect **VectPtr: a double pointer to the calee alocated vector of records			
			 * Return: true/false if there was a record to fetch or not	 
			 */
			virtual bool				FetchNextRow (GML::Utils::GTVector<GML::DB::DBRecord> &VectPtr)=0;

			/*
			 * Usage: fetch a new record after a previous SqlSelect call
			 * Param:
			 *	- INPUT/OUTPUT DbRecordVect **VectPtr: a double pointer to the calee alocated vector of records			
			 *	- INPUT UInt32 RowNr: the row number to be fetched
			 * Return: true/false if there was a record to fetch or not	 
			 */
			virtual bool				FetchRowNr (GML::Utils::GTVector<GML::DB::DBRecord> &VectPtr, UInt32 RowNr)=0;

			/*
			 *Usage: free the calee allocated vector of records given in a FetchRow call
			 *Param:
			 *	- INPUT DbRecordVect* Vect: a pointer to a DbRecordVect to be freed
			 *Return: true/false if the memory free succeded or not
			 */
			virtual bool				FreeRow(GML::Utils::GTVector<GML::DB::DBRecord> &Vect)=0;

			 /*
			  *Usage: insert a new ENTIRE row into the database
			  *Param:
			  *	- INPUT char* Table: the table the data is to be inserted in
			  *	- INPUT DbRecordVect * Vect: a vector of Record objects to be inserted
			  *	Return: true/false if the action succeded or not
			  */
			virtual bool				InsertRow (char* Table, GML::Utils::GTVector<GML::DB::DBRecord> &Vect)=0;

			/*
			  *Usage: insert a new ENTIRE row into the database
			  *Param:
			  *	- INPUT char* Table: the table the data is to be inserted in
			  *	- INPUT char* Fields: a string of fields to be inserted separated by comma
			  *	- INPUT DbRecordVect * Vect: a vector of Record objects to be inserted
			  *	Return: true/false if the action succeded or not
			  */
			virtual bool				InsertRow (char* Table, char* Fields, GML::Utils::GTVector<GML::DB::DBRecord> &Vect)=0;

			/*
			 * Usage: execute a sql update statement 
			 * Param:
			 *  - INPUT char* SqlStatement: the update sql statement
			 *  - INPUT DbRecordVect* WhereVals: the values used to create the where part of the statement
			 *  - INPUT DbRecordVect* UpdateVals: the values used to replace the old values
			 * Return: true/false if the operation succeded or not
			 */
			virtual bool				Update (char* SqlStatement, GML::Utils::GTVector<GML::DB::DBRecord> &WhereVals, GML::Utils::GTVector<GML::DB::DBRecord> &UpdateVals)=0;
		};
	}
}



//===================== MLInterface.h =================================================================================


namespace GML
{
	namespace ML
	{
		class IConector
		{
		protected:
			GML::Utils::INotify			*notifier;
			GML::DB::IDataBase			*database;
			GML::ML::IConector			*conector;

		public:	

			virtual bool				OnInit() = 0;
			bool						Init(GML::Utils::INotify &Notifier,GML::DB::IDataBase &Database);
			bool						Init(GML::ML::IConector &conector);

			/*	 
			 * Usage: uninit stuff
			 */
			virtual bool Close()=0;
	
			/*
			 *Usage: set the interval for this paralel unit's database
			 *Params:
			 *	-  UInt32 start: the start unit
			 *			!!! this parameter is 0 indexed (not 1)
			 *	-  UInt32 end: the end unit (this unit will not be included in the interval)	 
			 */
			virtual bool SetRecordInterval(UInt32 start, UInt32 end)=0;

			/*
			 * Usage: allocated, create and return a MLRecord structure instance
			 * Return: the allocated MlRecord structure or NULL if out of memory
			 */
			virtual bool CreateMlRecord (MLRecord &record)=0;

			/*
			 *Usage: Get a single record of data
			 *Params:
			 *	- OUTPUT MLRecord &record: the record to be fetched
			 *	- INPUT  UInt32 index: the record index
			 */
			virtual bool GetRecord(MLRecord &record,UInt32 index)=0;

			/*
			 * Usage: Free a MLRecord structure
			 * Params:
			 *	- INPUT MLRecord *record: a pointer to a structure received through a GetRecord call
			 */
			virtual bool FreeMLRecord(MLRecord &record)=0;


			/*
			 * Usage: Get the number of features 
			 */
			virtual UInt32 GetFeatureCount()=0;

			/*
			 * Usage:	- Get the number of records in the database	for the current paralel unit
			 *			- if no interval has been specified it returns the total number of records
			 */
			virtual UInt32 GetRecordCount()=0;
	
			/*
			 * Usage Get the total number of records in the database
			 */
			virtual UInt32 GetTotalRecordCount()=0;
		};

	}
}



//===================== Builder.h =================================================================================



namespace GML
{
	class  Builder
	{
	public:
		static GML::Utils::INotify*		CreateNotifyer(char *pluginName,void *objectData = NULL);	
		static GML::DB::IDataBase*		CreateDataBase(char *pluginName,GML::Utils::INotify &notify,char *connectionString);
	};
}


#endif

