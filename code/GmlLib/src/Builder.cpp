#include "Builder.h"

bool AdjustNameWithExtensionAndPath(GML::Utils::GString *path,char *extension)
{
	// verific daca are extensia care trebuie
	if (path->EndsWith(extension,true)==false)
	{
		if (path->Add(extension)==false)
			return false;
	}
	// verific daca are locatia care trebuie
	return true;
}

//==========================================================================================
GML::Utils::INotify*	GML::Builder::CreateNotifyer(char *pluginName,void *objectData)
{
	GML::Utils::GString		path;
	HMODULE					hModule;
	GML::Utils::INotify*	(*fnCreate)(void *objData);

	if (path.Set(pluginName)==false)
		return NULL;
	if (AdjustNameWithExtensionAndPath(&path,NOTIFYER_EXT)==false)
		return NULL;

	// incarc libraria
	if ((hModule = LoadLibraryA(path.GetText()))==INVALID_HANDLE_VALUE)
		return NULL;
	// incarc functia Create
	*(FARPROC *)&fnCreate = GetProcAddress(hModule,"Create");
	if (fnCreate==NULL)
		return NULL;
	// am incarcat si totul e ok -> cer o interfata
	return fnCreate(objectData);
}


