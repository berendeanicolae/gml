#include "INotifier.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define TEMP_STACK_BUFFER_SIZE		2048

bool GML::Utils::INotifier::NotifyString(UInt32 messageID,char *format,...)
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
bool GML::Utils::INotifier::Error(char *format,...)
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
		return Notify(GML::Utils::INotifier::NOTIFY_ERROR,stack,(unsigned int)len);
	} else {
		if ((temp = new char[len+1])==NULL)
			return false;

		while (true)
		{
			if ((len = vsprintf_s( temp, len+1, format, args ))<0)
				break;
			temp[len]=0;
			result = Notify(GML::Utils::INotifier::NOTIFY_ERROR,stack,(unsigned int)len);
			delete temp;
			return result;
		}
		delete temp;
		return false;
	}
}
bool GML::Utils::INotifier::Info(char *format,...)
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
		return Notify(GML::Utils::INotifier::NOTIFY_INFO,stack,(unsigned int)len);
	} else {
		if ((temp = new char[len+1])==NULL)
			return false;

		while (true)
		{
			if ((len = vsprintf_s( temp, len+1, format, args ))<0)
				break;
			temp[len]=0;
			result = Notify(GML::Utils::INotifier::NOTIFY_INFO,stack,(unsigned int)len);
			delete temp;
			return result;
		}
		delete temp;
		return false;
	}
}
bool GML::Utils::INotifier::StartProcent(char *format,...)
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
		return Notify(GML::Utils::INotifier::NOTIFY_START_PROCENT,stack,(unsigned int)len);
	} else {
		if ((temp = new char[len+1])==NULL)
			return false;

		while (true)
		{
			if ((len = vsprintf_s( temp, len+1, format, args ))<0)
				break;
			temp[len]=0;
			result = Notify(GML::Utils::INotifier::NOTIFY_START_PROCENT,stack,(unsigned int)len);
			delete temp;
			return result;
		}
		delete temp;
		return false;
	}
}
bool GML::Utils::INotifier::EndProcent()
{
	double proc = 1;
	Notify(NOTIFY_PROCENT,&proc,sizeof(double));
	return Notify(NOTIFY_END_PROCENT,"",0);
}
bool GML::Utils::INotifier::SetProcent(double procValue)
{
	return Notify(NOTIFY_PROCENT,&procValue,sizeof(double));
}
bool GML::Utils::INotifier::SetProcent(double procValue,double maxValue)
{
	return SetProcent(procValue/maxValue);
}
bool GML::Utils::INotifier::Init(char *attributeString)
{
	if ((attributeString!=NULL) && (attributeString[0]!=0))
	{
		if (SetProperty(attributeString)==false)
			return false;
	}
	return OnInit();
}
bool GML::Utils::INotifier::Result(GML::Utils::AlgorithmResult &ar)
{
	return Notify(NOTIFY_RESULT,&ar,sizeof(ar));
}
bool GML::Utils::INotifier::CreateObject(char *name,char *attributes)
{
	unsigned char Data[TEMP_STACK_BUFFER_SIZE];
	unsigned int  Size;

	if (name==NULL)
		return false;
	if (GML::Utils::GString::Set((char *)Data,name,31)==false)
		return false;
	Data[31]=0;
	if (attributes!=NULL)
	{
		for (Size=32;(Size<TEMP_STACK_BUFFER_SIZE-1) && ((*attributes)!=0);Size++,attributes++)
			Data[Size] = (*attributes);		
	} else {
		Size = 32;
	}
	Data[Size]=0;
	return Notify(NOTIFY_CREATEOBJECT,Data,Size);
}
bool GML::Utils::INotifier::SendDataToObject(char *objName,char *attributes)
{
	unsigned char Data[TEMP_STACK_BUFFER_SIZE];
	unsigned int  Size;

	if (objName==NULL)
		return false;
	if (GML::Utils::GString::Set((char *)Data,objName,31)==false)
		return false;
	Data[31]=0;
	if (attributes!=NULL)
	{
		for (Size=32;(Size<TEMP_STACK_BUFFER_SIZE-1) && ((*attributes)!=0);Size++,attributes++)
			Data[Size] = (*attributes);		
	} else {
		Size = 32;
	}
	Data[Size]=0;
	return Notify(NOTIFY_SENDOBJECTCOMMAND,Data,Size);
}