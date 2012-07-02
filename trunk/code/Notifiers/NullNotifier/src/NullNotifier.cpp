#include "stdio.h"
#include "NullNotifier.h"

NullNotifier::NullNotifier()
{
	ObjectName = "NullNotifier";
}
bool NullNotifier::OnInit()
{	
	return true;
}
bool NullNotifier::Uninit()
{
	return true;
}
bool NullNotifier::Notify(UInt32 messageID,void *Data,UInt32 DataSize)
{
	return true;
}
