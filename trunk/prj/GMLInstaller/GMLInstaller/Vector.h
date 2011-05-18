#pragma once

class Vector
{
private:
	unsigned int	NrElemente;
	unsigned int	ElementSize;
	unsigned int	AlocatedElements;
	unsigned char	*Data;

	bool			Grow();
public:
	Vector(void);
	~Vector(void);
	
	bool			Create(unsigned int alocElements=32,unsigned int ElemSize=sizeof(void *));  
	bool			Exists() { return (Data!=NULL); }
	unsigned int	GetSize() { return NrElemente; }
	bool			Push(void *Element);
	bool			Insert(void *Element,unsigned int index);
	void*			Get(unsigned int index);
	bool			CopyElement(unsigned int index,void *addr);
	bool			Delete(unsigned int index);
	void			DeleteAll();
	void			Resize(unsigned int newSmallSize);

};
