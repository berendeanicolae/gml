#include "stdio.h"
#include "WrapperNotifier.h"

WrapperNotifier::WrapperNotifier()
{
	ObjectName = "WrapperNotifier";

	hModule = NULL;
	fnInit = NULL;
	fnNotify = NULL;
	fnUninit = NULL;
	Context = NULL;

	LinkPropertyToString("Plugin",PluginPath,"","!!FILEPATH!!\nSpecifies the name of the dll plugin that will receive the messages !");	
}

bool WrapperNotifier::OnInit()
{	
	if (PluginPath.Len()==0)
	{
		DEBUGMSG("[%s] -> Missin plugin propery !",ObjectName);
		return false;
	}
	if ((hModule = LoadLibraryA(PluginPath.GetText()))==NULL)
	{
		DEBUGMSG("[%s] -> Unable to load : %s !",ObjectName,PluginPath.GetText());
		return false;
	}

	*(FARPROC *)&fnInit		= GetProcAddress(hModule,"Init");
	*(FARPROC *)&fnNotify	= GetProcAddress(hModule,"Notify");
	*(FARPROC *)&fnUninit	= GetProcAddress(hModule,"Uninit");

	if (fnInit==NULL)
	{
		DEBUGMSG("[%s] -> Plugin %s doesn`t export 'Init' function",ObjectName,PluginPath.GetText());
		return false;
	}
	if (fnNotify==NULL)
	{
		DEBUGMSG("[%s] -> Plugin %s doesn`t export 'Notify' function",ObjectName,PluginPath.GetText());
		return false;
	}
	if (fnUninit==NULL)
	{
		DEBUGMSG("[%s] -> Plugin %s doesn`t export 'Uninit' function",ObjectName,PluginPath.GetText());
		return false;
	}
	if ((Context = fnInit())==NULL)
	{
		DEBUGMSG("[%s] -> Init function returned a NULL context",ObjectName);
		return false;
	}
	// totul e ok , am un context
	return true;
}
bool WrapperNotifier::Uninit()
{
	if ((fnUninit!=NULL) && (Context!=NULL))
		fnUninit(Context);
	
	hModule = NULL;
	fnInit = NULL;
	fnNotify = NULL;
	fnUninit = NULL;
	Context = NULL;

	return true;
}
bool WrapperNotifier::Notify(UInt32 messageID,void *Data,UInt32 DataSize)
{
	if ((fnNotify!=NULL) && (Context!=NULL))
		return fnNotify(Context,messageID,Data,DataSize);
	DEBUGMSG("[%S] -> Unable to send message (Context=%p,NotifyFunction=%p)\n",ObjectName,Context,fnNotify);
	return false;
}
