#include "CSV.h"

CSV::CSV()
{
	ObjectName = "CSV";
	LinkPropertyToString("FileName",fileName,"","File that contains the database !");
}
bool	CSV::OnInit()
{
	return true;
}
bool	CSV::Connect ()
{
	GML::Utils::GString		token;
	int						poz,poz2;
	Column					c;

	Columns.DeleteAll();
	if (allDB.LoadFromFile(fileName.GetText())==false)
	{
		notifier->Error("[%s] -> Unable to open (%s) for reading !",ObjectName,fileName.GetText());
		return false;
	}
	if (allDB.CopyNextLine(&tempStr,&poz)==false)
	{
		notifier->Error("Error reading from %s",fileName.GetText());
		return false;
	}
	tempStr.Strip();
	// 
	poz2 = 0;
	while (tempStr.CopyNext(&token,",",&poz2))
	{
		token.Strip();
		GML::Utils::GString::Set(c.Name,token.GetText(),MAX_COLUMN_NAME);
		c.Type = GML::DB::TYPES::ASCII;

		if (Columns.PushByRef(c)==false)
		{
			notifier->Error("[%s] -> Unable to add '%s' to column names !",ObjectName,token.GetText());
			return false;
		}
	}
	if (Columns.Len()==0)
	{
		notifier->Error("[%s] -> first line in a csv file should be the header (found nothing) -> %s",ObjectName,tempStr.GetText());
		return false;
	}
	nrRecords = 0;
	while (allDB.CopyNextLine(&tempStr,&poz))
	{
		tempStr.Strip();
		if (tempStr.Len()>0)
			nrRecords++;
	}
	if (nrRecords==0)
	{
		notifier->Error("[%s] -> Missing records in %s",ObjectName,fileName.GetText());
		return false;
	}

	notifier->Info("[%s] -> CSV file loaded : %d records",ObjectName,nrRecords);
	return true;
}
bool	CSV::Disconnect ()
{
	return true;
}
bool	CSV::ExecuteQuery (char* Statement,UInt32 *rowsCount)
{
	GML::Utils::GString		tempStr;
	unsigned int			limit;
	// daca se cere numarul de inregistrari
	if (GML::Utils::GString::Find(Statement,"count",true)>=0)
	{
		modQueryCount = true;
		if (rowsCount)
			(*rowsCount) = 1;
		return true;
	}
	if (GML::Utils::GString::EndsWith(Statement,"limit 1",true))
	{
		limit = 0;
		if (rowsCount)
			(*rowsCount) = 1;
	} else {
		if (GML::Utils::GString::Find(Statement,"limit",true)>=0)
		{
			tempStr.Set(&Statement[GML::Utils::GString::Find(Statement,"limit",true)+5]);
			tempStr.Strip();
			if (tempStr.Contains(","))
				tempStr.Truncate(tempStr.Find(","));
			tempStr.Strip();
			if (tempStr.ConvertToUInt32(&limit)==false)
			{
				notifier->Error("[%s] -> Invalid number for limit: (%s) in '%s'",ObjectName,tempStr.GetText(),Statement);
				return false;
			}
		} else { 
			limit = 0; 
		}
	}
	modQueryCount = false;
	if (limit==0)
	{
		dbPoz = 0;
		// skipez prima linie (hederul)
		for (int tr=0;tr<1;tr++)
		{
			if (allDB.CopyNextLine(&tempStr,&dbPoz)==false)
			{
				notifier->Error("[%s] -> File::Error reading a line from %s",ObjectName,fileName.GetText());
				return false;
			}
		}
		cIndex = 0;
	} else {
		if (cIndex!=limit)
		{
			notifier->Error("[%s] -> Invalid position (current = %d , requested = %d) in '%s'",ObjectName,cIndex,limit,Statement);
			return false;
		}
	}
	if (rowsCount)
		(*rowsCount) = nrRecords;
	// totul e ok
	return true;
}
bool    CSV::GetColumnInformations(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr)
{
	GML::DB::DBRecord	rec;

	VectPtr.DeleteAll();

	for (UInt32 tr=0;tr<Columns.Len();tr++)
	{
		rec.Name = Columns[tr].Name;
		rec.Type = Columns[tr].Type;
		if (VectPtr.PushByRef(rec)==false)
		{
			notifier->Error("[%s] -> Unable to add column (%s) to Columns vectors",ObjectName,rec.Name);
			return false;
		}
	}	
	return true;
}

bool	CSV::FetchNextRow (GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr)
{
	GML::DB::DBRecord	rec;
	int					poz,tr;
	//UInt32				index;

	memset(&rec,0,sizeof(rec));

	if (VectPtr.DeleteAll()==false)
	{
		notifier->Error("[%s] -> Unable to delete all indexes from record vector",ObjectName);
		return false;
	}

	if (modQueryCount)
	{
		rec.Type = GML::DB::TYPES::UINT32;
		rec.Value.UInt32Val = nrRecords;
		if (VectPtr.PushByRef(rec)==false)
		{
			notifier->Error("[%s] -> Unable to add record count to vector !",ObjectName);
			return false;
		}
		return true;
	}

	while (true)
	{
		if (allDB.CopyNextLine(&tempStr,&dbPoz)==false)
			return false;
		tempStr.Strip();
		if (tempStr.Len()==0)
			continue;
		break;
	}
	
	cIndex++;
	poz = tr = 0;
	while (tempStr.CopyNext(&featureIndex,",",&poz))
	{
		rec.Name = Columns[tr].Name;
		rec.Type = Columns[tr].Type;
		// pun si valorile

		
	}

	//notifier->Info("Reading records: %d (%d)",cIndex,dbPoz);
	// adaug si un hash
	//rec.Name = "Hash";
	//rec.Type = GML::DB::HASHVAL;
	//rec.Hash.Reset();
	//
	//if (VectPtr.PushByRef(rec)==false)
	//{
	//	notifier->Error("Unable to add HASH to vector !");
	//	return false;
	//}
	/*
	// formatul este label:lista flaguri
	tempStr.Strip();
	tempStr.Replace(" ","");
	tempStr.Replace("\t","");
	rec.Name = "Label";
	rec.Type = GML::DB::DOUBLEVAL;
	if (tempStr.StartsWith("1:"))
	{
		rec.DoubleVal = 1.0;
		tempStr.ReplaceOnPos(0,2,"");
	} else {
		if (tempStr.StartsWith("-1:"))
		{
			rec.DoubleVal = -1.0;
			tempStr.ReplaceOnPos(0,3,"");
		} else {
			notifier->Error("Invalid format : %s",tempStr.GetText());
			return false;
		}
	}
	if (VectPtr.PushByRef(rec)==false)
	{
		notifier->Error("Unable to add label to vector !");
		return false;
	}
	// urc toate featurerile
	rec.Type = GML::DB::DOUBLEVAL;
	rec.DoubleVal = 0;
	for (int tr=0;tr<nrFeatures;tr++)
	{
		rec.Name = FeatNames[tr].GetText();
		if (VectPtr.PushByRef(rec)==false)
		{
			notifier->Error("Unable to add label to vector feature #%d => %s",tr,tempStr.GetText());
			return false;
		}
	}
	// am pus toate featurerile -> vad care efectiv sunt 1
	poz = 0;
	while (tempStr.CopyNext(&featureIndex,",",&poz))
	{
		if (featureIndex.ConvertToUInt32(&index)==false)
		{
			notifier->Error("Invalid value for feature index (%s) => %s",featureIndex.GetText(),tempStr.GetText());
			return false;
		}
		if (index>=nrFeatures)
		{
			notifier->Error("Index outside range[0..%d] (%d) => %s",nrFeatures,index,tempStr.GetText());
			return false;
		}
		// VectPtr[0] = Hash
		// VectPtr[1] = Label
		// VectPtr[2] = Feature[0]
		VectPtr[index+2].DoubleVal = 1.0;
	}	
	//*/
	return true;
}

bool	CSV::InsertRow (char* Table, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect)
{
	notifier->Error("[%s] -> InsertRow function not suported !",ObjectName);
	return false;
}
bool	CSV::InsertRow (char* Table, char* Fields, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect)
{
	notifier->Error("[%s] -> InsertRow function not suported !",ObjectName);
	return false;
}
bool	CSV::Update (char* SqlStatement, GML::Utils::GTFVector<GML::DB::DBRecord> &WhereVals, GML::Utils::GTFVector<GML::DB::DBRecord> &UpdateVals)
{
	notifier->Error("[%s] -> Update function not suported !",ObjectName);
	return false;
}