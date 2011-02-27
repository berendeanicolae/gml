// dllmain.cpp : Defines the entry point for the DLL application.

#include <Windows.h>
#include "gmllib.h"
#include "FullCacheConnector.h"

LIB_INTERFACE(FullCacheConnector,"MCU",1,"A connector that loads the entire database into memory as double for every record");


