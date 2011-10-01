// GML.cpp : Defines the entry point for the console application.
//

#include "gmllib.h"

void SetColor(unsigned char Fore, unsigned char Back)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),(Fore & 15)+(Back & 15)*16);
}
int  Error(char *message)
{
	SetColor(12,0);
	printf("[ERROR] : %s\n",message);
	SetColor(7,0);
	return -1;
}
void Print(char *text,int Fore,int back)
{
	SetColor(Fore,back);
	printf("%s",text);
}
void SaveToClipboard(char *text)
{
	int		tr,start=0,end;
	HANDLE	hMem;

	for (end=0;text[end]!=0;end++);
	end--;

	hMem=GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE,end-start+2);
	if (hMem!=NULL)
	{
		if (OpenClipboard(NULL))
		{
			EmptyClipboard();
			CloseClipboard();
		}
		char *temp=(char *)GlobalLock(hMem);
		if (temp!=NULL)
		{
				for (tr=start;tr<=end;tr++) temp[tr-start]=text[tr];
				temp[tr-start]=0;
		}
		GlobalUnlock(hMem);
		if (OpenClipboard(NULL) && (temp!=NULL))
		{
				HANDLE h=SetClipboardData(CF_TEXT,temp);
				DWORD res=GetLastError();
				CloseClipboard();
		}
	}
}
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
int  Run(char *templateName)
{
	GML::Utils::AttributeList	config;
	GML::Utils::GString			algName;
	GML::Algorithm::IAlgorithm	*alg;

	//printf("Run(%s)\n",templateName);
	if (config.Load(templateName)==false)
	{
		printf("%s\n",config.GetError());
		printf("[ERROR] -> unable to load properties from %s\n",templateName);
		return 2;
	}
	// aflu numele algoritmului
	if (config.UpdateString("AlgorithmName",algName)==false)
	{
		printf("[ERROR] -> Missing 'AlgorithmName' property from %s\n",templateName);
		return 3;
	}
	if ((alg=GML::Builder::CreateAlgorithm(algName.GetText()))==NULL)
	{
		printf("[ERROR] -> Error creating '%s'\n",algName.GetText());
		return 4;
	}
	if (alg->SetProperty(config)==false)
	{
		printf("[ERROR] -> Error updateing configuration to '%s'\n",algName.GetText());
		return 5;
	}
	if (alg->Init()==false)
	{
		printf("[ERROR] -> Error on initialization of '%s'\n",algName.GetText());
		return 6;
	}
	if (alg->Execute()==false)
	{
		printf("[ERROR] -> Error on execution of '%s'\n",algName.GetText());
		return 7;
	}
	alg->Wait();
	printf("Algorithm '%s' done !\n",algName.GetText());
	return 0;
}
int  Template(char *objectName,char *templateName)
{
	GML::Utils::AttributeList	attr;
	GML::Utils::Attribute		*a;
	GML::Utils::GString			tmp,result,desc,fName;
	GML::Utils::File			f;
	bool						isAlg = false;
	
	if (GML::Builder::GetPluginProperties(objectName,attr,&fName)==false)
	{
		printf("[ERROR] There is no algorithm, notifier, database or conector with the name : %s\n",objectName);
		return 1;
	}
	isAlg = fName.EndsWith(ALGORITHM_EXT,true);
	result.Set("");
	if (isAlg)
		result.AddFormatedEx("%{str,L30,F }= \"%{str}\"\n","AlgorithmName",objectName);
	else
		result.AddFormatedEx("\t%{str,L30,F }= \"%{str}\"\n","Type",objectName);

	for (UInt32 tr=0;tr<attr.GetCount();tr++)
	{
		if ((a=attr.Get(tr))==NULL)
			continue;
		tmp.Set("");
		if (isAlg)
			tmp.AddFormatedEx("%{str,L30,F }= ",a->Name);
		else
			tmp.AddFormatedEx("\t%{str,L30,F }= ",a->Name);
		switch (a->AttributeType)
		{
			case GML::Utils::AttributeList::BOOLEAN:
				if (*(bool *)a->Data)
					tmp.Add("True");
				else
					tmp.Add("False");
				break;
			case GML::Utils::AttributeList::UINT8:
				tmp.AddFormated("%u",*(UInt8 *)a->Data);
				break;
			case GML::Utils::AttributeList::UINT16:
				tmp.AddFormated("%u",*(UInt16 *)a->Data);
				break;
			case GML::Utils::AttributeList::UINT32:
				tmp.AddFormated("%u",*(UInt32 *)a->Data);
				break;
			case GML::Utils::AttributeList::INT8:
				tmp.AddFormated("%d",*(Int8 *)a->Data);
				break;
			case GML::Utils::AttributeList::INT16:
				tmp.AddFormated("%d",*(Int16 *)a->Data);
				break;
			case GML::Utils::AttributeList::INT32:
				tmp.AddFormated("%d",*(Int32 *)a->Data);
				break;
			case GML::Utils::AttributeList::FLOAT:
				tmp.AddFormated("%.3f",*(float *)a->Data);
				break;
			case GML::Utils::AttributeList::DOUBLE:
				tmp.AddFormated("%.3lf",*(double *)a->Data);
				break;
			case GML::Utils::AttributeList::STRING:
				tmp.AddFormated("\"%s\"",(char *)a->Data);
				break;
			default:
				tmp.AddFormated("<?Type=%d>",a->AttributeType);
				break;
		};
		desc.Set("");
		if (a->GetDescription(desc))
		{
			desc.Replace("\n",".");
			desc.Replace("\r",".");
			desc.Strip();
			if (desc.Len()>0)
				result.AddFormated("#%s\n",desc.GetText());
		}
		desc.Set("");
		if (((a->GetFlags() & GML::Utils::FL_LIST)!=0) && (a->AttributeType==GML::Utils::AttributeList::STRING))
		{
			if (a->GetListItems(desc))
				result.AddFormated("#one of [%s]\n",desc.GetText());
		}
		result.AddFormated("%s\n\n",tmp.GetText());		
	}
	if (isAlg==false)
		result.Add("}\n");
	if (templateName==NULL)
	{
		printf("%s\n",result.GetText());	
		SaveToClipboard(result.GetText());
	} else {
		if (f.Create(templateName)==false)
		{
			printf("[ERROR] Unable to create : %s\n",templateName);
			return 1;
		}
		if (f.Write(result.GetText(),result.Len())==false)
		{
			printf("[ERROR] Unable to write to file : %s\n",templateName);
			f.Close();
			DeleteFileA(templateName);
			return 1;
		}
		f.Close();
	}
	return 0;
}
int  PyTemplate(char *objectName)
{
	GML::Utils::AttributeList	attr;
	GML::Utils::Attribute		*a;
	GML::Utils::GString			tmp,result,desc,fullName;
	GML::Utils::File			f;
	
	if (GML::Builder::GetPluginProperties(objectName,attr,&fullName)==false)
	{
		printf("[ERROR] There is no algorithm, notifier, database or conector with the name : %s\n",objectName);
		return 1;
	}
	if (fullName.EndsWith(".alg",true))
		result.SetFormated("{\n\t\"AlgorithmName\":\"%s\",\n",objectName);
	else
		result.SetFormated("{\n\t\"Type\":\"%s\",\n",objectName);
	for (UInt32 tr=0;tr<attr.GetCount();tr++)
	{
		if ((a=attr.Get(tr))==NULL)
			continue;
		result.AddFormated("\t\"%s\":",a->Name);
		switch (a->AttributeType)
		{
			case GML::Utils::AttributeList::BOOLEAN:
				if (*(bool *)a->Data)
					result.Add("True");
				else
					result.Add("False");
				break;
			case GML::Utils::AttributeList::UINT8:
				result.AddFormated("%u",*(UInt8 *)a->Data);
				break;
			case GML::Utils::AttributeList::UINT16:
				result.AddFormated("%u",*(UInt16 *)a->Data);
				break;
			case GML::Utils::AttributeList::UINT32:
				result.AddFormated("%u",*(UInt32 *)a->Data);
				break;
			case GML::Utils::AttributeList::INT8:
				result.AddFormated("%d",*(Int8 *)a->Data);
				break;
			case GML::Utils::AttributeList::INT16:
				result.AddFormated("%d",*(Int16 *)a->Data);
				break;
			case GML::Utils::AttributeList::INT32:
				result.AddFormated("%d",*(Int32 *)a->Data);
				break;
			case GML::Utils::AttributeList::FLOAT:
				result.AddFormated("%.3f",*(float *)a->Data);
				break;
			case GML::Utils::AttributeList::DOUBLE:
				result.AddFormated("%.3lf",*(double *)a->Data);
				break;
			case GML::Utils::AttributeList::STRING:
				result.AddFormated("\"%s\"",(char *)a->Data);
				break;
			default:
				result.AddFormated("\"<?Type=%d>\"",a->AttributeType);
				break;
		};
		if ((tr+1)==attr.GetCount())
			result.Add("\n");
		else
			result.Add(",\n");
	}
	result.Add("}\n");
	printf("%s\n",result.GetText());	
	SaveToClipboard(result.GetText());
	return 0;
}
int  Info(char *objectName)
{
	GML::Utils::AttributeList	attr;
	GML::Utils::Attribute		*a;
	GML::Utils::GString			tmp,list;

	if (GML::Builder::GetPluginProperties(objectName,attr)==false)
	{
		printf("[ERROR] There is no algorithm, notifier, database or conector with the name : %s\n",objectName);
		return 1;
	}
	printf("[%s]\n---------------------------------------------------------------------------------\n",objectName);
	for (UInt32 tr=0;tr<attr.GetCount();tr++)
	{
		if ((a=attr.Get(tr))==NULL)
			continue;
		tmp.Set(a->Name);
		while (tmp.Len()<20)
			tmp.AddChar(' ');
		tmp.Add(" = ");
		switch (a->AttributeType)
		{
			case GML::Utils::AttributeList::BOOLEAN:
				if (*(bool *)a->Data)
					tmp.Add("True");
				else
					tmp.Add("False");
				break;
			case GML::Utils::AttributeList::UINT8:
				tmp.AddFormated("%u",*(UInt8 *)a->Data);
				break;
			case GML::Utils::AttributeList::UINT16:
				tmp.AddFormated("%u",*(UInt16 *)a->Data);
				break;
			case GML::Utils::AttributeList::UINT32:
				tmp.AddFormated("%u",*(UInt32 *)a->Data);
				break;
			case GML::Utils::AttributeList::INT8:
				tmp.AddFormated("%d",*(Int8 *)a->Data);
				break;
			case GML::Utils::AttributeList::INT16:
				tmp.AddFormated("%d",*(Int16 *)a->Data);
				break;
			case GML::Utils::AttributeList::INT32:
				tmp.AddFormated("%d",*(Int32 *)a->Data);
				break;
			case GML::Utils::AttributeList::FLOAT:
				tmp.AddFormated("%.3f",*(float *)a->Data);
				break;
			case GML::Utils::AttributeList::DOUBLE:
				tmp.AddFormated("%.3lf",*(double *)a->Data);
				break;
			case GML::Utils::AttributeList::STRING:
				tmp.AddFormated("'%s'",(char *)a->Data);
				if ((a->GetFlags() & GML::Utils::FL_LIST)!=0)
				{
					if (a->GetListItems(list))
						tmp.AddFormated(" one of [%s]",list.GetText());
				}
				break;
			default:
				tmp.AddFormated("<?Type=%d>",a->AttributeType);
				break;
		}
		printf("  %s\n",tmp.GetText());
	}
	printf("---------------------------------------------------------------------------------\n");
	return 0;
}
int  Desc(char *objectName,char *prop)
{
	GML::Utils::AttributeList	attr;
	GML::Utils::Attribute		*a;
	GML::Utils::GString			list;

	if (GML::Builder::GetPluginProperties(objectName,attr)==false)
	{
		printf("[ERROR] There is no algorithm, notifier, database or conector with the name : %s\n",objectName);
		return 1;
	}
	a = attr.Get(prop);
	if (a==NULL)
	{
		printf("[ERROR] Object '%s' does not have property '%s'\n",objectName,prop);
		return 1;
	}
	Print("Object     = ",7,0);SetColor(14,0);printf("%s\n",objectName);
	Print("Property   = ",7,0);SetColor(14,0);printf("%s\n",a->Name);
	Print("Type       = ",7,0);
	switch (a->AttributeType)
	{
			case GML::Utils::AttributeList::BOOLEAN:
				Print("Boolean\n",11,0);
				break;
			case GML::Utils::AttributeList::UINT8:
				Print("UInt8\n",11,0);
				break;
			case GML::Utils::AttributeList::UINT16:
				Print("UInt16\n",11,0);
				break;
			case GML::Utils::AttributeList::UINT32:
				Print("UInt32\n",11,0);
				break;
			case GML::Utils::AttributeList::INT8:
				Print("Int8\n",11,0);
				break;
			case GML::Utils::AttributeList::INT16:
				Print("Int16\n",11,0);
				break;
			case GML::Utils::AttributeList::INT32:
				Print("Int32\n",11,0);
				break;
			case GML::Utils::AttributeList::FLOAT:
				Print("Float\n",11,0);
				break;
			case GML::Utils::AttributeList::DOUBLE:
				Print("Double\n",11,0);
				break;
			case GML::Utils::AttributeList::STRING:
				Print("String\n",11,0);
				break;
			default:
				SetColor(12,0);printf("Unknown type = %d\n",a->AttributeType);
				break;
	}
	Print("List       = ",7,0);
	if ((a->GetFlags() & GML::Utils::FL_LIST)!=0)
	{
		
		if (a->GetListItems(list))
		{
			SetColor(14,0);	
			printf("one of [%s]\n",list.GetText());
		} else {
			Print("None\n",12,0);
		}
	} else {
		Print("None\n",12,0);
	}
	Print("Desciption = ",7,0);
	if ((a->GetDescription(list)) && (list.Len()>0))
	{
		Print(list.GetText(),15,0);
	} else {
		Print("None",12,0);
	}

	return 0;
}
int  ShowObjects(char *folder,char *ext)
{
	HANDLE					hFind;
	GML::Utils::GString		gmlLib,tmp;
	WIN32_FIND_DATA			dt;

	if (FindGMLLibPath(gmlLib)==false)
		return Error("Unable to locate gmllib.dll");
	
	tmp.SetFormated("*%s",ext);
	gmlLib.PathJoinName(folder);
	gmlLib.PathJoinName(tmp.GetText());
	//printf("%s\n",gmlLib.GetText());
	if ((hFind = FindFirstFile(gmlLib.GetText(),&dt))==INVALID_HANDLE_VALUE)
		return Error("FindFirstFile failed !");
	do
	{
		if (GML::Utils::GString::EndsWith(dt.cFileName,ext,true))
		{
			tmp.Set(dt.cFileName);
			tmp.Replace(ext,"");
			printf("  %s\n",tmp.GetText());
		}
	} while (FindNextFile(hFind,&dt));
	FindClose(&dt);
	return 0;
}
int  ShowHelp()
{
	Print("GML.EXE , Copyright @2011 , Gavrilut Dragos & co\n",15,0);	
	SetColor(14,0);printf("Build on %s (%s)\n",__DATE__,__TIME__);
	Print("Usage: ",6,0);Print("gml.exe command <parameters>\n",7,0);
	Print("Where: ",6,0);Print("command is one of the following\n",7,0);
	Print("       run ",10,0);Print("<template_file>           ",11,0);Print("-> executes a template file\n",12,0);
	Print("       info ",10,0);Print("<object_name>            ",11,0);Print("-> shows informations about a specific Algorithm,Conector,DataBase or Notifier\n",12,0);	
	Print("       desc ",10,0);Print("<object_name> <property> ",11,0);Print("-> shows description for a specify propery from an Algorithm,Conector,DataBase or Notifier\n",12,0);	
	Print("       template ",10,0);Print("<object> [file]      ",11,0);Print("-> creates a template for a specifiy object. If no file is present , it copies to clipboard the template\n",12,0);	
	Print("       pytemplate ",10,0);Print("<object>           ",11,0);Print("-> creates and copies to clipboard a template for a specifiy object\n",12,0);	
	Print("       algorithms ",10,0);Print("                   ",11,0);Print("-> shows a list of existing algorithms\n",12,0);
	Print("       connectors ",10,0);Print("                   ",11,0);Print("-> shows a list of existing connectors\n",12,0);
	Print("       notifiers ",10,0);Print("                    ",11,0);Print("-> shows a list of existing notifiers\n",12,0);
	Print("       databases ",10,0);Print("                    ",11,0);Print("-> shows a list of existing databases\n",12,0);

	SetColor(7,0);
	printf("\n");
	return 0;
}
int  main(int argc, char* argv[])
{
	if (argc<=1)
		return ShowHelp();
	// run
	if (GML::Utils::GString::Equals(argv[1],"run",true))
	{
		if (argc!=3)
			return Error("run command requare a parameter (a template file)");
		return Run(argv[2]);
	}
	// info
	if (GML::Utils::GString::Equals(argv[1],"info",true))
	{
		if (argc!=3)
			return Error("info command requare a parameter (a name for a an Algorithm, Conector, DataBase or Notifier)");
		return Info(argv[2]);
	}
	// desc
	if (GML::Utils::GString::Equals(argv[1],"desc",true))
	{
		if (argc!=4)
			return Error("desc command requare two parameters (a name for a an Algorithm, Conector, DataBase or Notifier and a property)");
		return Desc(argv[2],argv[3]);
	}
	// template
	if (GML::Utils::GString::Equals(argv[1],"template",true))
	{
		if ((argc!=4) && (argc!=3))
			return Error("template command requares a name for a an Algorithm, Conector, DataBase or Notifier");
		if (argc==3)
			return Template(argv[2],NULL);
		else
			return Template(argv[2],argv[3]);
	}
	// pytemplate
	if (GML::Utils::GString::Equals(argv[1],"pytemplate",true))
	{
		if (argc!=3)
			return Error("pytemplate command requares one parameter (a name for a an Algorithm, Conector, DataBase or Notifier)");
		return PyTemplate(argv[2]);
	}
	// algorithms
	if (GML::Utils::GString::Equals(argv[1],"algorithms",true))
		return ShowObjects(ALGORITHM_FOLDER,ALGORITHM_EXT);
	// connectors
	if (GML::Utils::GString::Equals(argv[1],"connectors",true))
		return ShowObjects(CONNECTOR_FOLDER,CONNECTOR_EXT);
	// notifiers
	if (GML::Utils::GString::Equals(argv[1],"notifiers",true))
		return ShowObjects(NOTIFYER_FOLDER,NOTIFYER_EXT);
	// databases
	if (GML::Utils::GString::Equals(argv[1],"databases",true))
		return ShowObjects(DATABASE_FOLDER,DATABASE_EXT);

	printf("[ERROR] Unknwon command : %s \n",argv[1]);
	return -1;
}
