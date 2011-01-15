
#include "ConsoleNotifier.h"

bool ConsoleNotifier::Init(void * data)
{
	return true;
}

bool ConsoleNotifier::Notify( char* msg )
{
	printf (msg);
	return true;
}

bool ConsoleNotifier::UnInit()
{
	return true;
}
