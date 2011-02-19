#include "INotify.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define TEMP_STACK_BUFFER_SIZE		2048

bool GML::Utils::INotify::NotifyString(UInt32 messageID,char *format,...)
{
    va_list		args;
    int			len;
	char		stack[TEMP_STACK_BUFFER_SIZE];
	char		*temp;
	bool		result;

    va_start( args, format );
	if ((len = _vscprintf( format, args ))<0)
		return false;
	
	if (len+1<TEMP_STACK_BUFFER_SIZE)
	{
		if ((len = vsprintf_s( stack, len+1, format, args ))<0)
			return false;
		stack[len]=0;
		return Notify(messageID,stack,(unsigned int)len);
	} else {
		if ((temp = new char[len+1])==NULL)
			return false;

		while (true)
		{
			if ((len = vsprintf_s( temp, len+1, format, args ))<0)
				break;
			temp[len]=0;
			result = Notify(messageID,stack,(unsigned int)len);
			delete temp;
			return result;
		}
		delete temp;
		return false;
	}
}
bool GML::Utils::INotify::Error(char *format,...)
{
    va_list		args;
    int			len;
	char		stack[TEMP_STACK_BUFFER_SIZE];
	char		*temp;
	bool		result;

    va_start( args, format );
	if ((len = _vscprintf( format, args ))<0)
		return false;
	
	if (len+1<TEMP_STACK_BUFFER_SIZE)
	{
		if ((len = vsprintf_s( stack, len+1, format, args ))<0)
			return false;
		stack[len]=0;
		return Notify(GML::Utils::INotify::NOTIFY_ERROR,stack,(unsigned int)len);
	} else {
		if ((temp = new char[len+1])==NULL)
			return false;

		while (true)
		{
			if ((len = vsprintf_s( temp, len+1, format, args ))<0)
				break;
			temp[len]=0;
			result = Notify(GML::Utils::INotify::NOTIFY_ERROR,stack,(unsigned int)len);
			delete temp;
			return result;
		}
		delete temp;
		return false;
	}
}
bool GML::Utils::INotify::Info(char *format,...)
{
    va_list		args;
    int			len;
	char		stack[TEMP_STACK_BUFFER_SIZE];
	char		*temp;
	bool		result;

    va_start( args, format );
	if ((len = _vscprintf( format, args ))<0)
		return false;
	
	if (len+1<TEMP_STACK_BUFFER_SIZE)
	{
		if ((len = vsprintf_s( stack, len+1, format, args ))<0)
			return false;
		stack[len]=0;
		return Notify(GML::Utils::INotify::NOTIFY_INFO,stack,(unsigned int)len);
	} else {
		if ((temp = new char[len+1])==NULL)
			return false;

		while (true)
		{
			if ((len = vsprintf_s( temp, len+1, format, args ))<0)
				break;
			temp[len]=0;
			result = Notify(GML::Utils::INotify::NOTIFY_INFO,stack,(unsigned int)len);
			delete temp;
			return result;
		}
		delete temp;
		return false;
	}
}
bool GML::Utils::INotify::Init(char *attributeString)
{
	Attr.Clear();

	if ((attributeString!=NULL) && (attributeString[0]!=0))
	{
		if (Attr.Create(attributeString)==false)			
			return false;
	}
	return OnInit();
}



