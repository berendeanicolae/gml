//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#include "imp_msgbox.h"
#include <AtlBase.h>
#include <AtlConv.h>
//---------------------------------------------------------------------------
void __imp_message_box_custom(int type, char *caption, char *format, ...)
{
	char msg[512];

	va_list args;
	va_start(args, format);
	vsprintf_s(msg, 512, format, args);
	va_end(args);

#ifdef _UNICODE
	MessageBox(0, CT2CW((LPCTSTR)msg), CT2CW((LPCTSTR)caption), type);
#else
	MessageBox(0, msg, caption, type);
#endif
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
