#ifndef __RESTRICTION_MANAGER_MESSAGE_BOX_H__
#define __RESTRICTION_MANAGER_MESSAGE_BOX_H__
//--------------------------------------------------------------------------------------------------
#include "imp_defines.h"
//--------------------------------------------------------------------------------------------------
#ifdef __IMP_MESSAGE_BOX
//--------------------------------------------------------------------------------------------------
void __imp_message_box_custom(int type, char *caption, char *format, ...);
#define __imp_message_box(type, caption, text)									(__imp_message_box_custom(0, text, caption, type))
#define __imp_message_box_f(type, caption, format, ...)					(__imp_message_box_custom(type, caption, format, __VA_ARGS__))
//--------------------------------------------------------------------------------------------------
#else//__IMP_MESSAGE_BOX
//--------------------------------------------------------------------------------------------------
#define __imp_message_box(type, caption, text)									((void)0)
#define __imp_message_box_f(type, caption, format, ...)					((void)0)
//--------------------------------------------------------------------------------------------------
#endif//__PVM_MESSAGE_BOX
//--------------------------------------------------------------------------------------------------
#endif//__RESTRICTION_MANAGER_MESSAGE_BOX_H__
