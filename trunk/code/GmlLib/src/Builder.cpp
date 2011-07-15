#include "Builder.h"
#include "GString.h"

struct PluginObjectInfo
{
	GML::Utils::GString			templateText;
	GML::Utils::GString			typeName;
	GML::Utils::AttributeList	attr;
	GML::Utils::TemplateParser	tp;	
	UInt32						CountElements;
};

bool FindGMLLibPath(GML::Utils::GString &gmlLib)
{
	HMODULE					hModule;
	GML::Utils::GString		tmp;
	char					temp[2048];

	
	if ((hModule = GetModuleHandleA("gmllib.dll"))==NULL)
		return false;
	if (GetModuleFileNameA(hModule,temp,2048)==0)
		return false;
	if (tmp.Set(temp)==false)
		return false;
	if (tmp.CopyPathName(&gmlLib)==false)
		return false;

	return true;
}
bool AdjustNameWithExtensionAndPath(GML::Utils::GString &path,char *extension,char *folderName)
{
	// verific daca are extensia care trebuie
	if (path.EndsWith(extension,true)==false)
	{
		if (path.Add(extension)==false)
			return false;
	}
	// verific daca are locatia care trebuie
	if (path.Contains("\\")==false) // e doar numele
	{
		GML::Utils::GString		gmlLibPath;
		if (FindGMLLibPath(gmlLibPath)==false)
			return false;
		if (gmlLibPath.PathJoinName(folderName)==false)
			return false;
		if (gmlLibPath.PathJoinName(&path)==false)
			return false;

		if (path.Set(&gmlLibPath)==false)
			return false;
	}

	return true;
}
bool CheckIfArray(GML::Utils::GString &str,GML::Utils::INotifier *notifier,PluginObjectInfo &poi)
{
	UInt32	tip;

	poi.CountElements = 0;
	
	if (str.Strip()==false)
	{
		if (notifier)
			notifier->Error("[GML:Builder] -> Internal error -> GString::Strip(%s)",str.GetText());
		return false;
	}
	if (str.StartsWith("["))
	{
		if (str.EndsWith("]")==false)
		{
			if (notifier)
				notifier->Error("[GML:Builder] -> Missing ']' in %s",str.GetText());
			return false;
		}
		// este o lista -> il parsez
		if (poi.tp.Parse(&str.GetText()[1],str.Len()-2)==false)
		{
			if (notifier)
				notifier->Error(poi.tp.GetError());
			return false;
		}
		// verific sa fie formata doar din dictionare
		if (poi.tp.GetCount()==0)
		{
			if (notifier)
				notifier->Error("[GML:Builder] ->Empty list : %s",str.GetText());
			return false;
		}
		for (UInt32 tr=0;tr<poi.tp.GetCount();tr++)
		{
			tip = poi.tp.Tokens[tr].Type;
			if (tip==GML::Utils::TemplateParser::TOKEN_DICT)
			{
				poi.CountElements++;
				continue;
			}
			if (tip==GML::Utils::TemplateParser::TOKEN_TERM)
				continue;
			
			if (notifier)
				notifier->Error("[GML:Builder] -> List should contain only dictionaries : ",str.GetText());
			return false;			
		}
		if (poi.CountElements == 0)
		{
			if (notifier)
				notifier->Error("[GML:Builder] -> List should contain at least one dictionary : ",str.GetText());
			return false;
		}
		// totul e ok , am un array
	}
	return true;
}
bool CreatePluginObjectInfo(char *text,GML::Utils::INotifier *notifier,PluginObjectInfo &poi)
{
	if (poi.templateText.Set(text)==false)
	{
		if (notifier)
			notifier->Error("[GML:Builder] -> Invalid template : %s",text);
		return false;
	}
	if (poi.templateText.Strip()==false)
	{
		if (notifier)
			notifier->Error("[GML:Builder] -> Internal error -> GString::Strip(%s)",text);
		return false;
	}
	if (poi.templateText.StartsWith("{"))
	{
		if (poi.templateText.EndsWith("}")==false)
		{
			if (notifier)
				notifier->Error("[GML:Builder] -> Missing '}' in %s",text);
			return false;
		}
		if (poi.templateText.DeleteChar(0)==false)
		{
			if (notifier)
				notifier->Error("[GML:Builder] -> Internal error -> GString::DeleteChar(%s)",poi.templateText.GetText());
			return false;
		}
		poi.templateText.Truncate(poi.templateText.Len()-1);
	}
	if (poi.attr.Create(poi.templateText.GetText())==false)
	{
		if (notifier)
		{
			notifier->Error(poi.attr.GetError());
			notifier->Error("[GML:Builder] -> Invalid template: %s",poi.templateText.GetText());
		}
		return false;
	}
	if (poi.attr.UpdateString("type",poi.typeName)==false)
	{
		if (notifier)
			notifier->Error("[GML:Builder] -> Missing 'Type' property in : ",poi.templateText.GetText());
		return false;
	}
	return true;
}
//==========================================================================================
GML::Utils::INotifier*		GML::Builder::CreateNotifier(char *buildString)
{
	GML::Utils::GString		notif;
	HMODULE					hModule;	
	GML::Utils::INotifier*	(*fnCreate)();
	GML::Utils::INotifier*	newObject;
	PluginObjectInfo		poi;

	if (CreatePluginObjectInfo(buildString,NULL,poi)==false)
		return NULL;
	if (AdjustNameWithExtensionAndPath(poi.typeName,NOTIFYER_EXT,NOTIFYER_FOLDER)==false)
		return NULL;

	// incarc libraria
	if ((hModule = LoadLibraryA(poi.typeName.GetText()))==NULL)
		return NULL;
	// incarc functia Create
	*(FARPROC *)&fnCreate = GetProcAddress(hModule,"CreateInterface");
	if (fnCreate==NULL)
		return NULL;
	// am incarcat si totul e ok -> cer o interfata
	if ((newObject=fnCreate())==NULL)
		return NULL;
	if (newObject->Init(poi.templateText.GetText())==false)
		return NULL;
	// totul e ok -> verific daca nu cumva mai are si un alt notificator
	if ((poi.attr.UpdateString("Notifier",notif)) && (notif.Len()>2))
	{
		newObject->Notifier = GML::Builder::CreateNotifier(notif.GetText());
		if (newObject->Notifier==NULL)
			return NULL;
	}
	return newObject;
}
GML::DB::IDataBase*			GML::Builder::CreateDataBase(char *buildString,GML::Utils::INotifier &notify)
{	
	HMODULE					hModule;	
	GML::DB::IDataBase*		(*fnCreate)();
	GML::DB::IDataBase*		newObject;
	PluginObjectInfo		poi;

	if (CreatePluginObjectInfo(buildString,&notify,poi)==false)
		return NULL;
	if (AdjustNameWithExtensionAndPath(poi.typeName,DATABASE_EXT,DATABASE_FOLDER)==false)
		return NULL;
	// incarc libraria
	if ((hModule = LoadLibraryA(poi.typeName.GetText()))==NULL)
	{
		notify.Error("[GML:Builder] -> Unable to load library: %s",poi.typeName.GetText());
		return NULL;
	}
	// incarc functia Create
	*(FARPROC *)&fnCreate = GetProcAddress(hModule,"CreateInterface");
	if (fnCreate==NULL)
	{
		notify.Error("[GML:Builder] -> Missing 'CreateInterface' export from library: %s",poi.typeName.GetText());
		return NULL;
	}
	// am incarcat si totul e ok -> cer o interfata
	if ((newObject=fnCreate())==NULL)
	{
		notify.Error("[GML:Builder] -> 'CreateInterface' failed in library: %s",poi.typeName.GetText());
		return NULL;
	}
	if (newObject->Init(notify,poi.templateText.GetText())==false)
	{
		notify.Error("[GML:Builder] -> 'Init' failed in library: %s",poi.typeName.GetText());
		return NULL;
	}
	return newObject;
}
GML::ML::IConnector*		GML::Builder::CreateConnector(char *buildString,GML::Utils::INotifier &notify)
{	
	UInt32					tip,index;
	GML::Utils::GString		conn;
	HMODULE					hModule;	
	GML::ML::IConnector*	(*fnCreate)();
	GML::ML::IConnector*	newObject;
	PluginObjectInfo		poi;

	if (CreatePluginObjectInfo(buildString,&notify,poi)==false)
		return NULL;
	if (AdjustNameWithExtensionAndPath(poi.typeName,CONNECTOR_EXT,CONNECTOR_FOLDER)==false)
		return NULL;
	// incarc libraria
	if ((hModule = LoadLibraryA(poi.typeName.GetText()))==NULL)
	{
		notify.Error("[GML:Builder] -> Unable to load library: %s",poi.typeName.GetText());
		return NULL;
	}
	// incarc functia Create
	*(FARPROC *)&fnCreate = GetProcAddress(hModule,"CreateInterface");
	if (fnCreate==NULL)
	{
		notify.Error("[GML:Builder] -> Missing 'CreateInterface' export from library: %s",poi.typeName.GetText());
		return NULL;
	}
	// am incarcat si totul e ok -> cer o interfata
	if ((newObject=fnCreate())==NULL)
	{
		notify.Error("[GML:Builder] -> 'CreateInterface' failed in library: %s",poi.typeName.GetText());
		return NULL;
	}
	if ((poi.attr.UpdateString("Connector",conn)) && (conn.Len()>2))
	{
		// legatura cu ceilalti connectori
		if (CheckIfArray(conn,&notify,poi)==false)
			return NULL;
		if (poi.CountElements>0)
		{
			// array
			if ((newObject->connectors = new GML::ML::IConnector* [poi.CountElements])==NULL)
			{
				notify.Error("[GML:Builder] -> Unable to alloc %d connectors for : %s",poi.CountElements,poi.typeName.GetText());
				return NULL;
			}
			newObject->connectorsCount = poi.CountElements;
			index = 0;
			for (UInt32 tr=0;tr<poi.tp.GetCount();tr++)
			{
				if (poi.tp.Tokens[tr].Type != GML::Utils::TemplateParser::TOKEN_DICT)
					continue;
				if (poi.tp.Get(tr,conn,tip)==false)
				{
					notify.Error("[GML:Builder] -> Unable to read connector(%d) from array : %s",index,poi.typeName.GetText());
					return NULL;
				}
				newObject->connectors[index] = GML::Builder::CreateConnector(conn.GetText(),notify);
				if (newObject->connectors[index]==NULL)
					return NULL;
			}
		} else {
			if ((newObject->connectors = new GML::ML::IConnector* [1])==NULL)
			{
				notify.Error("[GML:Builder] -> Unable to alloc connectors for : %s",poi.typeName.GetText());
				return NULL;
			}
			newObject->connectorsCount = 1;
			newObject->connectors[0] = GML::Builder::CreateConnector(conn.GetText(),notify);
			if (newObject->connectors[0]==NULL)
				return NULL;
		}
		// fac si linkul pt. connector
		newObject->conector = newObject->connectors[0];
	}
	if ((poi.attr.UpdateString("DataBase",conn)) && (conn.Len()>2))
	{
		if (newObject->connectorsCount>0)
		{
			notify.Error("[GML:Builder] -> A connector cannot be connected at the same time to a DataBase and another connector : %s",poi.typeName.GetText());
			return NULL;
		}
		newObject->database = GML::Builder::CreateDataBase(conn.GetText(),notify);
		if (newObject->database == NULL)
			return NULL;
	}
	if (newObject->Init(notify,poi.templateText.GetText())==false)
	{
		notify.Error("[GML:Builder] -> 'Init' failed in library: %s",poi.typeName.GetText());
		return NULL;
	}
	return newObject;
}
GML::Algorithm::IAlgorithm*	GML::Builder::CreateAlgorithm(char *algorithmPath)
{
	GML::Utils::GString			path;
	HMODULE						hModule;
	GML::Algorithm::IAlgorithm*	(*fnCreate)();

	if (path.Set(algorithmPath)==false)
		return NULL;
	if (AdjustNameWithExtensionAndPath(path,ALGORITHM_EXT,ALGORITHM_FOLDER)==false)
		return NULL;

	// incarc libraria
	if ((hModule = LoadLibraryA(path.GetText()))==NULL)
		return NULL;
	// incarc functia Create
	*(FARPROC *)&fnCreate = GetProcAddress(hModule,"CreateInterface");
	if (fnCreate==NULL)
		return NULL;
	// am incarcat si totul e ok -> cer o interfata
	return fnCreate();
}
bool						GML::Builder::GetPluginProperties(char *pluginName,GML::Utils::AttributeList &attr,GML::Utils::GString *fullName)
{
	GML::Utils::GString		path;
	HMODULE					hModule;
	bool					(*fnGetInterfaceProperty)(GML::Utils::AttributeList &attr);	
	char					*ext[4];
	char					*folders[4];


	ext[0]=ALGORITHM_EXT;	folders[0]=ALGORITHM_FOLDER;
	ext[1]=DATABASE_EXT;	folders[1]=DATABASE_FOLDER;
	ext[2]=CONNECTOR_EXT;	folders[2]=CONNECTOR_FOLDER;
	ext[3]=NOTIFYER_EXT;	folders[3]=NOTIFYER_FOLDER;

	for (int tr=0;tr<4;tr++)
	{
		if (path.Set(pluginName)==false)
			continue;
		if (AdjustNameWithExtensionAndPath(path,ext[tr],folders[tr])==false)
			continue;
		if ((hModule = LoadLibraryA(path.GetText()))==NULL)
			continue;
		*(FARPROC *)&fnGetInterfaceProperty = GetProcAddress(hModule,"GetInterfaceProperty");
		if (fnGetInterfaceProperty==NULL)
			continue;
		if (fullName!=NULL)
			fullName->Set(&path);
		return fnGetInterfaceProperty(attr);
	}
	return false;
}

