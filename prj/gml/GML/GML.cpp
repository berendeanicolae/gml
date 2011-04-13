// GML.cpp : Defines the entry point for the console application.
//

#include "gmllib.h"

int  Error(char *message)
{
	printf("[ERROR] : %s\n",message);
	return -1;
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

	if (config.Load(templateName)==false)
	{
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
	GML::Utils::GString			tmp,result,desc;
	GML::Utils::File			f;
	
	if (GML::Builder::GetPluginProperties(objectName,attr)==false)
	{
		printf("[ERROR] There is no algorithm, notifier, database or conector with the name : %s\n",objectName);
		return 1;
	}
	result.SetFormated("AlgorithmName        = %s\n;----------------------------------------------------------------------------\n\n",objectName);
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
				result.AddFormated(";%s\n",desc.GetText());
		}
		desc.Set("");
		if (((a->GetFlags() & GML::Utils::FL_LIST)!=0) && (a->AttributeType==GML::Utils::AttributeList::STRING))
		{
			if (a->GetListItem(desc))
				result.AddFormated(";one of [%s]\n",desc.GetText());
		}
		result.AddFormated("%s\n\n",tmp.GetText());		
	}
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
					if (a->GetListItem(list))
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
	printf("GML.EXE , Copyright @2011 , Gavrilut Dragos & co\n");
	printf("Build on %s (%s)\n",__DATE__,__TIME__);
	printf("Usage: gml.exe command <parameters>\n");
	printf("Where: command is one of the following\n");
	printf("       run <template_file>       -> executes a template file\n");
	printf("       info <object_name>        -> shows informations about a specific Algorithm,Conector,DataBase or Notifier\n");
	printf("       template <object> <file>  -> creates a template for a specifiy object\n");
	printf("       algorithms                -> shows a list of existing algorithms\n");
	printf("       connectors                -> shows a list of existing connectors\n");
	printf("       notifiers                 -> shows a list of existing notifiers\n");
	printf("       databases                 -> shows a list of existing databases\n");

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
	// template
	if (GML::Utils::GString::Equals(argv[1],"template",true))
	{
		if (argc!=4)
			return Error("template command requare two parameters (a name for a an Algorithm, Conector, DataBase or Notifier and a file name)");
		return Template(argv[2],argv[3]);
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