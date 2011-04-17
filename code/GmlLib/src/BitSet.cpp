#include "BitSet.h"


GML::Utils::BitSet::BitSet()
{
	Data = NULL;
	Allocated = 0;
	ElementsCount = 0;
}
GML::Utils::BitSet::~BitSet()
{
	Destroy();
}
bool	GML::Utils::BitSet::Create(UInt32 elements)
{
	UInt32	alloc;

	Destroy();
	if ((elements % 8)==0)
		alloc = elements / 8;
	else
		alloc = (elements / 8)+1;
	if ((Data = new UInt8[alloc])==NULL)
		return false;
	ElementsCount = elements;
	Allocated = alloc;
	memset(Data,0,alloc);
	return true;
}
void	GML::Utils::BitSet::Destroy()
{
	if (Data!=NULL)
		delete Data;
	Data = NULL;
	Allocated = 0;
	ElementsCount = 0;
}
bool	GML::Utils::BitSet::Get(UInt32 poz)
{
	if (poz>=ElementsCount)
		return false;
	return (bool)((Data[poz /8] & (1<<(poz % 8)))!=0);
}
bool	GML::Utils::BitSet::Set(UInt32 poz,bool value)
{
	UInt8	*p;
	UInt8	bit;
	if (poz>=ElementsCount)
		return false;
	p = &Data[poz / 8];
	bit = (1<<(poz % 8));
	if (value)
		(*p) |= bit;
	else
		(*p) &= (!bit);

	return true;
}
void	GML::Utils::BitSet::SetAll(bool value)
{
	if ((ElementsCount==0) || (Data == NULL))
		return;
	if (value)
		memset(Data,1,Allocated);
	else
		memset(Data,0,Allocated);
}
bool	GML::Utils::BitSet::Reverse(UInt32 poz)
{
	UInt8	*p;
	UInt8	bit;
	if (poz>=ElementsCount)
		return false;
	p = &Data[poz / 8];
	bit = (1<<(poz % 8));
	if (((*p) & bit)!=0)
		(*p) -= bit;
	else
		(*p) |= bit;

	return true;
}
void	GML::Utils::BitSet::ReverseAll()
{
	UInt32	tr;
	UInt8	*p;
	if (Allocated>0)
	{
		for (tr=0,p=Data;tr<Allocated;tr++,p++)
		{
			(*p) = !(*p);
		}
	}
}
UInt32	GML::Utils::BitSet::Len()
{
	return ElementsCount;
}
UInt32	GML::Utils::BitSet::GetAllocated()
{
	return Allocated;
}
UInt8*	GML::Utils::BitSet::GetData()
{
	return Data;
}