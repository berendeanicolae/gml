#include "StdAfx.h"
#include "Vector.h"

struct tmpEl { void *element; };

Vector::Vector(void)
{
	NrElemente=ElementSize=AlocatedElements=0;
	Data=NULL;
}

Vector::~Vector(void)
{
	DeleteAll();
	NrElemente=ElementSize=AlocatedElements=0;
	Data=NULL;
}
bool Vector::Grow()
{
	unsigned char *tmp;
	if (Data==NULL) return false;

	tmp=Data;
	Data=new unsigned char [AlocatedElements * ElementSize * 2];
	if (Data==NULL) { Data=tmp;return false; }
	memcpy(Data,tmp,AlocatedElements * ElementSize);
	AlocatedElements*=2;

	delete tmp;
	return true;
}
bool Vector::Create(unsigned int alocElements,unsigned int ElemSize)
{
	if (Data!=NULL) return false;
	if ((alocElements<1) || (ElemSize==0)) return false;
	
	NrElemente=0;
	AlocatedElements=alocElements;
	ElementSize=ElemSize;

	Data=new unsigned char [AlocatedElements * ElementSize];
	return true;
}
bool Vector::Push(void *Element)
{
	if (Data==NULL) return false;

	if ((NrElemente>=AlocatedElements) && (!Grow())) return false;
	memcpy(&Data[NrElemente*ElementSize],Element,ElementSize);
	NrElemente++;

	return true;
}
bool Vector::Insert(void *Element,unsigned int index)
{
	if (Data==NULL) return false;
	if ((NrElemente>=AlocatedElements) && (!Grow())) return false;
	if (index<NrElemente)
	{
		memmove(&Data[(index+1)*ElementSize],&Data[index*ElementSize],(NrElemente-index)*ElementSize);
	}
	memcpy(&Data[index*ElementSize],Element,ElementSize);
	NrElemente++;
	return true;
}
bool Vector::CopyElement(unsigned int index,void *addr)
{
	if ((index>NrElemente) || (addr==NULL) || (Data==NULL)) return false;
	memcpy(addr,&Data[index *ElementSize],ElementSize);
	return true;
}
void*Vector::GetPointer(unsigned int index)
{
	void* element = Get(index);
	if(element == NULL)
		return NULL;

	return ((tmpEl *)element)->element;

//	return el->element;
}

void*Vector::Get(unsigned int index)
{
	void *tmp=NULL;
	if ((index>=NrElemente) || (Data==NULL)) return NULL;
	return &Data[index * ElementSize];
}

bool Vector::Delete(unsigned int index)
{
	if ((index>NrElemente) || (Data==NULL)) return false;
	
	// sterg efectiv elementele
	if (index+1<NrElemente)
	{
		memcpy(&Data[index*ElementSize],&Data[(index+1)*ElementSize],(NrElemente-(index+1))*ElementSize);
	}
	NrElemente--;
	return true;
}
void Vector::Resize(unsigned int newSmallSize)
{
	if (Data==NULL) return;
	if (newSmallSize<NrElemente) NrElemente=newSmallSize;
}
void Vector::DeleteAll()
{
	if (Data==NULL) return;
	NrElemente=0;
}
