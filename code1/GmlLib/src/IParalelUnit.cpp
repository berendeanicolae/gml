#include "IParalelUnit.h"

bool GML::Utils::IParalelUnit::Init(unsigned int _ID,void *_context, void (*_fnThread)(IParalelUnit *,void *))
{
	context = _context;
	fnThread = _fnThread;
	ID = _ID;
	codeID = 0;
	return true;
}
unsigned int	GML::Utils::IParalelUnit::GetID()
{
	return ID;
}
unsigned int	GML::Utils::IParalelUnit::GetCodeID()
{
	return codeID;
}