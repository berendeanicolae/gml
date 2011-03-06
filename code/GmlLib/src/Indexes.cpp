#include "Indexes.h"

GML::Utils::Interval::Interval()
{
	Start = End = 0;
}
void GML::Utils::Interval::Set(UInt32 _start,UInt32 _end)
{
	if (_start<=_end)
	{
		Start = _start;
		End = _end;
	} else {
		Start = _end;
		End = _start;
	}
}
UInt32 GML::Utils::Interval::Size()
{
	return End-Start;
}
GML::Utils::Indexes::Indexes()
{
	List = NULL;
	MaxAlloc = 0;
	Count = 0;
}
GML::Utils::Indexes::~Indexes()
{
	Destroy();
}
			
bool    GML::Utils::Indexes::Create(UInt32 count)
{
	Destroy();
	if (count<1)
		return false;
	if ((List = new UInt32[count])==NULL)
		return false;
	Count = 0;
	MaxAlloc = count;
	return true;

}
void    GML::Utils::Indexes::Destroy()
{
	if (List!=NULL)
		delete List;
	List = NULL;
	Count = 0;
	MaxAlloc = 0;
}
			
bool    GML::Utils::Indexes::Push(UInt32 index)
{
	if (Count>=MaxAlloc)
		return false;
	List[Count++] = index;
	return true;
}
UInt32  GML::Utils::Indexes::Get(UInt32 poz)
{
	if (poz>=Count)
		return INVALID_INDEX;
	return List[poz];
}
UInt32*	GML::Utils::Indexes::GetList()
{
	return List;
}
UInt32	GML::Utils::Indexes::Len()
{
	return Count;
}
UInt32	GML::Utils::Indexes::GetTotalAlloc()
{
	return MaxAlloc;
}
bool	GML::Utils::Indexes::Truncate(UInt32 newCount)
{
	if (newCount>Count)
		return false;
	Count = newCount;
	return true;
}
bool	GML::Utils::Indexes::CopyTo(GML::Utils::Indexes &idx)
{
	if (Count==0)
		return false;
	if (idx.MaxAlloc<Count)
	{
		if (idx.Create(Count)==false)
			return false;
	}
	memcpy(idx.List,List,sizeof(UInt32)*Count);
	idx.Count = Count;

	return true;
}