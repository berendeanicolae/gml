#ifndef __CONSOLENOTIFIER_H
#define __CONSOLENOTIFIER_H

#include "gml.h"
#include <stdio.h>

class ConsoleNotifier : public INotifier
{
public:

	virtual bool Init(void * data);
	virtual bool Notify(char* msg);	
	virtual bool UnInit();

};

#endif