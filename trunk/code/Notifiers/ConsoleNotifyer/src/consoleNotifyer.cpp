#include "stdio.h"
#include "ConsoleNotifyer.h"


bool ConsoleNotifyer::Init(void *initData)
{
	return true;
}
bool ConsoleNotifyer::Uninit()
{
	return true;
}
bool ConsoleNotifyer::Notify(UInt32 messageID,void *Data,UInt32 DataSize)
{
	char *text = (char *)Data;
	printf("%4d => ",messageID);
	if ((Data!=NULL) && (DataSize>0))
	{
		for (UInt32 tr =0;tr<DataSize;tr++)
		{
			if ((text[tr]>=' ') && (text[tr]<=128))
				printf("%c",text[tr]);
			else
				printf("{\\x%02X}",(unsigned char)text[tr]);
		}
	}
	printf("\n");
	return true;
}
