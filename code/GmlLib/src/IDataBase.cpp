#include "IDataBase.h"

GML::DB::IDataBase::IDataBase()
{
	ObjectName = "IDataBase";
	Names.Create(2048);
	Columns.Create(1024);
	nrRecords = 0;
}
UInt32 GML::DB::IDataBase::GetRecordCount()
{
	return nrRecords;
}
GML::Utils::GTFVector<GML::DB::ColumnInfo>*	GML::DB::IDataBase::GetColumns()
{
	return &Columns;
}
bool GML::DB::IDataBase::AddColumn(UInt32 DataType,UInt32 ColumnType,char *name)
{
	GML::DB::ColumnInfo		ci;

	ci.DataType = DataType;
	ci.ColumnType = ColumnType;
	ci.Name = NULL;

	if (name==NULL)
	{
		GML::Utils::GString		str;
		if (str.SetFormated("Column_%d",Columns.Len()+1)==false)
		{
			notifier->Error("[%s] -> Internal error (unable to create string for column #%d) ",ObjectName,Columns.Len()+1);
			return false;
		}
		name = str.GetText();
	}
	ci.NameIndex = Names.Len();
	name--;
	do
	{
		name++;
		if (Names.PushByRef((char)(*name))==false)
		{
			notifier->Error("[%s] -> Internal error (unable to create string for column #%d)",ObjectName,Columns.Len()+1);
			return false;
		}
	} while ((*name)!=0);
	// daca totul e ok , adaug si in lista de coloane
	if (Columns.PushByRef(ci)==false)
	{
		notifier->Error("[%s] -> Unable to add column #%d to list.",ObjectName,Columns.Len()+1);
		return false;
	}
	return true;
}
bool GML::DB::IDataBase::Init(GML::Utils::INotifier &_notifier, char *connectionString)
{
	GML::Utils::GString		temp;

	notifier = &_notifier;

	if ((connectionString!=NULL) && (connectionString[0]!=0))
	{
		if (SetProperty(connectionString)==false)
		{
			notifier->Error("[%s] -> Invalid format for DataBase initializations: %s",ObjectName,connectionString);
			return false;
		}
	}
	if ((Names.DeleteAll()==false) || (Columns.DeleteAll()==false))
	{
		notifier->Error("[%s] -> Internal error: Unable to reset Names/Columns vectors ",ObjectName);
		return false;
	}
	// am creat atributele
	notifier->Info("[%s] -> OnInit()",ObjectName);
	if (OnInit()==false)
	{
		notifier->Error("[%s] -> OnInit() returned false",ObjectName);
		return false;
	}
	if (nrRecords==0)
	{
		notifier->Error("[%s] -> NULL number of records on database.",ObjectName);
		return false;
	}
	if (Columns.Len()==0)
	{
		notifier->Error("[%s] -> NULL number of columns on database.",ObjectName);
		return false;
	}
	// refac pointerii pentru coloane
	for (UInt32 tr=0;tr<Columns.Len();tr++)
	{
		Columns[tr].Name = (char *)Names.GetPtrToObject(Columns[tr].NameIndex);
		if (Columns[tr].Name == NULL)
		{
			notifier->Error("[%s] -> Internal error (Column #%d has no name).",ObjectName,tr+1);
			return false;
		}
	}
	temp.Set("");
	temp.AddFormatedEx("[%{str}] -> Init ok (Records = %{uint32,dec,G3} , Columns = %{uint32,dec,G3} , NameSize = %{uint32,dec,G3} bytes)",ObjectName,nrRecords,Columns.Len(),Names.Len());
	notifier->Info("%s",temp.GetText());

	return true;
}