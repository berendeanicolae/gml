#ifndef __RESTRICTION_MANAGER_ASSERT_H__
#define __RESTRICTION_MANAGER_ASSERT_H__
//---------------------------------------------------------------------------
#include "imp_msgbox.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//---------------------------------------------------------------------------
#ifdef __IMP_ASSERT
//---------------------------------------------------------------------------
//trebuie sa fie debifata optiunea Tools->Options->Debugger Options->CodeGear Debuggers->Ignore Non User Breakpoints
#define _on_assertion_fail							_CrtDbgBreak()
//#define _on_assertion_fail						exit(EXIT_FAILURE)
void imp_assertion_failed_message(char* file, int line, char* expr);
#define imp_assert(expr)								((expr) ? void(0) : (imp_assertion_failed_message(__FILE__, __LINE__, #expr)))
//---------------------------------------------------------------------------
#else	//__IMP_ASSERT
//---------------------------------------------------------------------------
#define imp_assert(expr)								void(0)
//---------------------------------------------------------------------------
#endif//__IMP_ASSERT
//---------------------------------------------------------------------------
#endif//__RESTRICTION_MANAGER_ASSERT_H__
