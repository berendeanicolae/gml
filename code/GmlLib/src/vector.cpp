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
		for (int tr=0;tr<ElementSize;tr++,e1++,e2++)
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
bool GML::Utils::Vector::Create(unsigned int alocElements,unsigned int ElemSize)
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
void GML::Utils::Vector::Sort(_BinarySearchCompFunction cmpFunc,bool ascendet)
{
	if (NrElemente>0)
		__QSort(Data,ElementSize,cmpFunc,0,NrElemente-1,ascendet);
}
