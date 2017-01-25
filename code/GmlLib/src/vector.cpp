#include "Vector.h"

int  _Do_BinarySearch(unsigned char *Data,int ElementSize,void *Element,_BinarySearchCompFunction compFunction,int lo,int hi)
{
	int mij;
	int res;

	do
	{
		mij=(lo+hi)/2;
		if ((res=compFunction(Element,&Data[mij*ElementSize]))==0) return mij;
		if (res<0) hi=mij-1;
		if (res>0) lo=mij+1;
	} while ((lo<=hi) && (hi>=0));
	return -1;
}
int  _Get_InsertSort_Locus(unsigned char *Data,int ElementSize,void *Element,_BinarySearchCompFunction compFunction,int lo,int hi,bool ascending)
{
	int mij;
	int res;
	if(ascending == false){//redundant code for fater execution
		do
		{
			mij=(lo+hi)/2;
			if ((res=-compFunction(Element,&Data[mij*ElementSize]))==0) return mij;
			if (res<0) hi=mij-1;
			if (res>0) lo=mij+1;
		} while ((lo<=hi) && (hi>=0));
	}
	else{
		do
		{
			mij=(lo+hi)/2;
			if ((res=compFunction(Element,&Data[mij*ElementSize]))==0) return mij;
			if (res<0) hi=mij-1;
			if (res>0) lo=mij+1;
		} while ((lo<=hi) && (hi>=0));
	}
	

	if( res<0 ) return lo;
	else return hi+1;
}
void __ElementsSwap(unsigned char *e1,unsigned char *e2,unsigned int ElementSize)
{
	unsigned char Temp[1024];
	
	if (ElementSize<1024)
	{
		memcpy(Temp,e1,ElementSize);
		memcpy(e1,e2,ElementSize);
		memcpy(e2,Temp,ElementSize);
	} else {
		for (unsigned int tr=0;tr<ElementSize;tr++,e1++,e2++)
		{
			Temp[0] = (*e1);
			(*e1) = (*e2);
			(*e2) = Temp[0];
		}
	}
}
void __QSort(unsigned char *Data,int ElementSize,_BinarySearchCompFunction cmpFunc,int lo,int hi,bool ascendent)
{
	Int32				left,right,mid;
	unsigned char		*pivot;	

	if (lo>=hi) 
		return;
	//DEBUGMSG("QSORT:(%d,%d)\n",lo,hi);
	left=lo;
	right=hi;
	mid=(lo+hi)/2;
	__ElementsSwap(&Data[left*ElementSize],&Data[mid*ElementSize],ElementSize);
	pivot=&Data[left*ElementSize];
	lo++;

	while (lo<=hi)
	{
		if (ascendent)
		{
			while ((lo<=right) && (cmpFunc(&Data[lo*ElementSize],pivot)<=0))
				++lo;
			while ((hi>=left) &&  (cmpFunc(&Data[hi*ElementSize],pivot)>0))
				--hi;
		} else {
			while ((lo<=right) && (cmpFunc(&Data[lo*ElementSize],pivot)>=0))
				++lo;
			while ((hi>=left) &&  (cmpFunc(&Data[hi*ElementSize],pivot)<0))
				--hi;
		}
		if (lo<hi)
		{
			__ElementsSwap(&Data[lo*ElementSize],&Data[hi*ElementSize],ElementSize);

		}
	} 
	__ElementsSwap(&Data[left*ElementSize],&Data[hi*ElementSize],ElementSize);

	// apelurile recursive
	__QSort(Data,ElementSize,cmpFunc,left,hi-1,ascendent);
	__QSort(Data,ElementSize,cmpFunc,hi+1,right,ascendent);
}
void __BubbleSort(unsigned char *Data,int ElementSize,_BinarySearchCompFunction cmpFunc,int nrElements,bool ascendent)
{
	Int32			tr,lastChange,lastElement;
	unsigned char	*e1,*e2;
	bool			sorted;

	if (nrElements<2)
		return;

	if (ascendent)
	{
		lastChange = nrElements;
		do
		{
			sorted = true;		
			lastElement = lastChange-1;
			for (tr=0;tr<lastElement;tr++)
			{
				e1 = &Data[tr*ElementSize];
				e2 = (e1+ElementSize);
				if (cmpFunc(e1,e2)>0)
				{
					__ElementsSwap(e1,e2,ElementSize);
					lastChange = tr+1;
					sorted = false;
				}
			}
		} while (!sorted);
	} else {
		lastChange = nrElements;
		do
		{
			sorted = true;		
			lastElement = lastChange-1;
			for (tr=0;tr<lastElement;tr++)
			{
				e1 = &Data[tr*ElementSize];
				e2 = (e1+ElementSize);
				if (cmpFunc(e1,e2)<0)
				{
					__ElementsSwap(e1,e2,ElementSize);
					lastChange = tr+1;
					sorted = false;
				}
			}
		} while (!sorted);
	}
}
void __HeapSort(unsigned char *Data,int ElementSize,_BinarySearchCompFunction cmpFunc,int nrElements,bool ascendent) 
{
    unsigned char	*tempElement;	// echivalentul lui t
	unsigned char	tempData[1024];
    unsigned int	n = nrElements, parent = nrElements/2, index, child; 

	if (ElementSize<=1024)
	{
		tempElement = tempData;
	} else {
		if ((tempElement = new unsigned char[ElementSize])==NULL)
			return;
	}

	if (ascendent)
	{
		for (;;) { 
			if (parent > 0) { 
				memcpy(tempElement,&Data[(--parent)*ElementSize],ElementSize);
			} else {
				n--;                /* make the heap smaller */
				if (n == 0) return; /* When the heap is empty, we are done */
				memcpy(tempElement,&Data[n*ElementSize],ElementSize);
				memcpy(&Data[n*ElementSize],&Data[0],ElementSize);
			}
			index = parent; /* start at the parent index */
			child = index * 2 + 1; /* get its left child index */
			while (child < n) {
				if ((child + 1 < n) && (cmpFunc(&Data[(child+1)*ElementSize],&Data[child*ElementSize])>0))
					child++;

				if (cmpFunc(&Data[child*ElementSize],tempElement)>0)
				{
					memcpy(&Data[ElementSize*index],&Data[ElementSize*child],ElementSize);
					index = child; 
					child = index * 2 + 1; 
				} else {
					break;
				}
			}
			memcpy(&Data[index*ElementSize],tempElement,ElementSize);
		}
	} else {
		for (;;) { 
			if (parent > 0) { 
				memcpy(tempElement,&Data[(--parent)*ElementSize],ElementSize);
			} else {
				n--;                /* make the heap smaller */
				if (n == 0) return; /* When the heap is empty, we are done */
				memcpy(tempElement,&Data[n*ElementSize],ElementSize);
				memcpy(&Data[n*ElementSize],&Data[0],ElementSize);
			}
			index = parent; /* start at the parent index */
			child = index * 2 + 1; /* get its left child index */
			while (child < n) {
				if ((child + 1 < n) && (cmpFunc(&Data[(child+1)*ElementSize],&Data[child*ElementSize])<0))
					child++;

				if (cmpFunc(&Data[child*ElementSize],tempElement)<0)
				{
					memcpy(&Data[ElementSize*index],&Data[ElementSize*child],ElementSize);
					index = child; 
					child = index * 2 + 1; 
				} else {
					break;
				}
			}
			memcpy(&Data[index*ElementSize],tempElement,ElementSize);
		}
	}
	if (ElementSize>1024)
		delete tempElement;
}
void __HeapSortContext(unsigned char *Data,int ElementSize,_ContextCompFunction cmpFunc,int nrElements,bool ascendent,void *Context) 
{
    unsigned char	*tempElement;	// echivalentul lui t
	unsigned char	tempData[1024];
    unsigned int	n = nrElements, parent = nrElements/2, index, child; 

	if (ElementSize<=1024)
	{
		tempElement = tempData;
	} else {
		if ((tempElement = new unsigned char[ElementSize])==NULL)
			return;
	}

	if (ascendent)
	{
		for (;;) { 
			if (parent > 0) { 
				memcpy(tempElement,&Data[(--parent)*ElementSize],ElementSize);
			} else {
				n--;                /* make the heap smaller */
				if (n == 0) return; /* When the heap is empty, we are done */
				memcpy(tempElement,&Data[n*ElementSize],ElementSize);
				memcpy(&Data[n*ElementSize],&Data[0],ElementSize);
			}
			index = parent; /* start at the parent index */
			child = index * 2 + 1; /* get its left child index */
			while (child < n) {
				if ((child + 1 < n) && (cmpFunc(&Data[(child+1)*ElementSize],&Data[child*ElementSize],Context)>0))
					child++;

				if (cmpFunc(&Data[child*ElementSize],tempElement,Context)>0)
				{
					memcpy(&Data[ElementSize*index],&Data[ElementSize*child],ElementSize);
					index = child; 
					child = index * 2 + 1; 
				} else {
					break;
				}
			}
			memcpy(&Data[index*ElementSize],tempElement,ElementSize);
		}
	} else {
		for (;;) { 
			if (parent > 0) { 
				memcpy(tempElement,&Data[(--parent)*ElementSize],ElementSize);
			} else {
				n--;                /* make the heap smaller */
				if (n == 0) return; /* When the heap is empty, we are done */
				memcpy(tempElement,&Data[n*ElementSize],ElementSize);
				memcpy(&Data[n*ElementSize],&Data[0],ElementSize);
			}
			index = parent; /* start at the parent index */
			child = index * 2 + 1; /* get its left child index */
			while (child < n) {
				if ((child + 1 < n) && (cmpFunc(&Data[(child+1)*ElementSize],&Data[child*ElementSize],Context)<0))
					child++;

				if (cmpFunc(&Data[child*ElementSize],tempElement,Context)<0)
				{
					memcpy(&Data[ElementSize*index],&Data[ElementSize*child],ElementSize);
					index = child; 
					child = index * 2 + 1; 
				} else {
					break;
				}
			}
			memcpy(&Data[index*ElementSize],tempElement,ElementSize);
		}
	}
	if (ElementSize>1024)
		delete tempElement;
}
//===================================================================================================================
GML::Utils::Vector::Vector(void)
{
	NrElemente=ElementSize=AlocatedElements=0;	
	Data=NULL;
}

void  GML::Utils::Vector::operator=(Vector& v)
{
	Free(); // sa nu avem leak-uri :P
	if (v.NrElemente>0)
	{
		NrElemente = v.NrElemente;
		ElementSize = v.ElementSize;
		AlocatedElements=v.AlocatedElements;
		if ((this->Data = new unsigned char [v.AlocatedElements*ElementSize])==NULL)
		{
			Free();
		} else {		
			memcpy(Data,v.Data,AlocatedElements * ElementSize);
		}
	}
}

GML::Utils::Vector::~Vector(void)
{
	Free();
}
bool GML::Utils::Vector::Grow(unsigned int newSize)
{
	unsigned char *tmp;

	if (Data==NULL) 
		return false;
	if (newSize==0xFFFFFFFF) newSize=AlocatedElements * 2;
	tmp=Data;
	if ((Data=new unsigned char [newSize*ElementSize])==NULL)
	{ 
		Data=tmp;
		return false; 
	}
	memcpy(Data,tmp,AlocatedElements * ElementSize);
	AlocatedElements=newSize;

	delete tmp;
	return true;
}
void GML::Utils::Vector::Free()
{
	if (Data!=NULL) delete Data;
	NrElemente=ElementSize=AlocatedElements=0;	
	Data=NULL;
}
bool GML::Utils::Vector::Create(unsigned int alocElements,unsigned int ElemSize,bool resizeToAllocElements)
{
	Free();
	if ((alocElements<1) || (ElemSize==0)) 
		return false;
	NrElemente=0;
	AlocatedElements=alocElements;
	ElementSize=ElemSize;
	if ((Data=new unsigned char [AlocatedElements * ElementSize])==NULL)
	{
		Free();
		return false;
	}
	if (resizeToAllocElements)
		NrElemente = AlocatedElements;
	return true;
}
bool GML::Utils::Vector::Push(void *Element)
{
	if (Data==NULL)
		return false;
	if ((NrElemente>=AlocatedElements) && (!Grow())) 
		return false;
	memcpy(&Data[NrElemente*ElementSize],Element,ElementSize);
	NrElemente++;
	return true;
}
bool GML::Utils::Vector::Insert(void *Element,unsigned int index)
{
	if (Data==NULL)
		return false;
	if ((NrElemente>=AlocatedElements) && (!Grow())) 
		return false;
	if (index<NrElemente)
	{
		memmove(&Data[(index+1)*ElementSize],&Data[index*ElementSize],(NrElemente-index)*ElementSize);
	}
	memcpy(&Data[index*ElementSize],Element,ElementSize);
	NrElemente++;
	return true;
}
bool GML::Utils::Vector::Insert(void *Element,_BinarySearchCompFunction compFunc,bool ascendent)
{
	unsigned int index = 0;

	// gasesti indexul in functie de cmpFunc si ascendent
	if(NrElemente == 0){
		return Insert(Element,0);
	}
	return Insert(Element,_Get_InsertSort_Locus(Data,ElementSize,Element,compFunc,0,NrElemente-1,ascendent));
}
bool GML::Utils::Vector::CopyElement(unsigned int index,void *addr)
{
	if ((index>NrElemente) || (addr==NULL)) 
		return false;
	memcpy(addr,&Data[index *ElementSize],ElementSize);
	return true;
}
void*GML::Utils::Vector::Get(unsigned int index)
{
	if ((Data==NULL) || (index>=NrElemente))
		return NULL;
	return &Data[index * ElementSize];
}

bool GML::Utils::Vector::Delete(unsigned int index)
{
	if ((index>NrElemente) || (Data==NULL)) 
		return false;
	// sterg efectiv elementele
	if (index+1<NrElemente)
	{
		memcpy(&Data[index*ElementSize],&Data[(index+1)*ElementSize],(NrElemente-(index+1))*ElementSize);
	}
	NrElemente--;
	return true;
}
bool GML::Utils::Vector::Resize(unsigned int newSize)
{
	// NU TREBUIE SCHIMBAT COMPORTAMENTUL -> E UTIL CAND INCARC DIN FISIER DIRECT IN BUFFER
	if (Data==NULL) 
		return false;
	if ((newSize>AlocatedElements) && (Grow(newSize)==false))
		return false;
	NrElemente=newSize;
	return true;
}
bool GML::Utils::Vector::DeleteAll()
{
	return Resize(0);
}
int  GML::Utils::Vector::BinarySearch (void *ElementToFind,_BinarySearchCompFunction compFnc)
{
	if ((ElementToFind==NULL) || (Data==NULL) || (NrElemente==0))
		return -1;
	else
		return _Do_BinarySearch(Data,ElementSize,ElementToFind,compFnc,0,NrElemente-1);
}
bool  GML::Utils::Vector::EqualRange (void *ElementToFind,_BinarySearchCompFunction compFnc,int *left_location,int *right_location)
{
    int poz;
    
	if ((ElementToFind==NULL) || (Data==NULL) || (NrElemente==0))
		return false;
    if ((poz=_Do_BinarySearch(Data,ElementSize,ElementToFind,compFnc,0,NrElemente-1))==-1)
        return false;
        
    if (left_location!=NULL)
    {
        *left_location = poz-1;
        while (((*left_location)>=0) && (compFnc(&Data[*left_location*ElementSize],ElementToFind)==0))
            (*left_location)--;
        (*left_location)++;
    }
    // merg in dreapta daca e nevoie
    if (right_location!=NULL)
    {
        *right_location = poz+1;
        while (((*right_location)<(Int32)NrElemente) && (compFnc(&Data[*right_location*ElementSize],ElementToFind)==0))
            (*right_location)++;
        (*right_location)--;
    }
    
    return true;
}
void GML::Utils::Vector::Sort(_BinarySearchCompFunction cmpFunc,bool ascendet)
{
	if (NrElemente>0)
		//__QSort(Data,ElementSize,cmpFunc,0,NrElemente-1,ascendet);
		//__BubbleSort(Data,ElementSize,cmpFunc,NrElemente,ascendet);
		__HeapSort(Data,ElementSize,cmpFunc,NrElemente,ascendet);
}
void GML::Utils::Vector::Sort(_ContextCompFunction cmpFunc,bool ascendet,void *Context)
{
	if (NrElemente>0)
		__HeapSortContext(Data,ElementSize,cmpFunc,NrElemente,ascendet,Context);
}
