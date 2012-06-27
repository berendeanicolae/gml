//---------------------------------------------------------------------------
#include "imp_assert.h"
#include <AtlBase.h>
#include <AtlConv.h>
//---------------------------------------------------------------------------
#ifdef __IMP_ASSERT
//---------------------------------------------------------------------------
void imp_assertion_failed_message(char* file, int line, char *expr)
{
	char displayed_message[256];

	displayed_message[0] = '\0';

	sprintf_s(displayed_message, 256, "%s, File : %s Line : %d", expr, file, line);
	__imp_message_box_f(MB_OK, "Assertion Failure!", displayed_message);
	//MessageBox(0, displayed_message, L"Assertion Failure", MB_OK);

	_on_assertion_fail;
}
//---------------------------------------------------------------------------
#endif//__IMP_ASSERT
//---------------------------------------------------------------------------
