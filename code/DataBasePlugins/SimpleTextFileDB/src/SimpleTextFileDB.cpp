#include "SimpleTextFileDB.h"

SimpleTextFileDB::SimpleTextFileDB()
{
	FeatNames = NULL;
	ObjectName = "SimpleTextFileDB";
	LinkPropertyToString("FileName",fileName,"","File that contains the database !");
}
bool	SimpleTextFileDB::OnInit()
{
	return true;
}
bool	SimpleTextFileDB::Connect ()
{
	int poz = 0;
	if (allDB.LoadFromFile(fileName.GetText())==false)
	{
		notifier->Error("Unable to open (%s) for reading !",fileName.GetText());
		return false;
	}
	if (allDB.CopyNextLine(&tempStr,&poz)==false)
	{
		notifier->Error("Error reading from %s",fileName.GetText());
		return false;
	}
	tempStr.Strip();
	if (tempStr.StartsWith("Records",true)==false)
	{
		notifier->Error("Expecting Records = <value> as first line in %s",fileName.GetText());
		return false;
	}
	tempStr.Replace("Records","",true);
	tempStr.Replace("=","");
	tempStr.Strip();
	if ((tempStr.ConvertToUInt32(&nrRecords)==false) || (nrRecords<1))
	{
		notifier->Error("Invalid numeric value for records number: %s",tempStr.GetText());
		return false;
	}
	if (allDB.CopyNextLine(&tempStr,&poz)==false)
	{
		notifier->Error("Error reading from %s",fileName.GetText());
		return false;
	}
	tempStr.Strip();
	if (tempStr.StartsWith("Features",true)==false)
	{
		notifier->Error("Expecting Features = <value> as first line in %s",fileName.GetText());
		return false;
	}
	tempStr.Replace("Features","",true);
	tempStr.Replace("=","");
	tempStr.Strip();
	if ((tempStr.ConvertToUInt32(&nrFeatures)==false) || (nrFeatures<1))
	{
		notifier->Error("Invalid numeric value for features number: %s",tempStr.GetText());
		return false;
	}
	if ((FeatNames = new GML::Utils::GString[nrFeatures])==NULL)
		return false;
	for (UInt32 tr = 0;tr<nrFeatures;tr++)
	{
		if (FeatNames[tr].SetFormated("Feat_%d",tr)==false)
			return false;
	}
	notifier->Info("Data Base loaded : %d records , %d features ",nrRecords,nrFeatures);
	return true;
}
bool	SimpleTextFileDB::Disconnect ()
{
	//file.Close();
	if (FeatNames!=NULL)
		delete []FeatNames;
	FeatNames = NULL;
	return true;
}
bool	SimpleTextFileDB::ExecuteQuery (char* Statement,UInt32 *rowsCount)
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
		// skipez primele 2 linii
		for (int tr=0;tr<2;tr++)
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
bool    SimpleTextFileDB::GetColumnInformations(GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr)
{
	GML::DB::DBRecord	rec;

	VectPtr.DeleteAll();

	rec.Name = "Hash";
	rec.Type = GML::DB::HASHVAL;
	
	if (VectPtr.PushByRef(rec)==false)
	{
		notifier->Error("Unable to add HASH to vector !");
		return false;
	}
	rec.Name = "Label";
	rec.Type = GML::DB::DOUBLEVAL;
	if (VectPtr.PushByRef(rec)==false)
	{
		notifier->Error("Unable to add label to vector !");
		return false;
	}
	rec.Type = GML::DB::DOUBLEVAL;	
	for (int tr=0;tr<nrFeatures;tr++)
	{
		rec.Name = FeatNames[tr].GetText();
		if (VectPtr.PushByRef(rec)==false)
		{
			notifier->Error("Unable to add label to vector feature #%d => %s",tr,tempStr.GetText());
			return false;
		}
	}	
	return true;
}

bool	SimpleTextFileDB::FetchNextRow (GML::Utils::GTFVector<GML::DB::DBRecord> &VectPtr)
{
	GML::DB::DBRecord	rec;
	int					poz;
	UInt32				index;

	memset(&rec,0,sizeof(rec));

	if (VectPtr.DeleteAll()==false)
	{
		notifier->Error("[%s] -> Unable to delete all indexes from record vector",ObjectName);
		return false;
	}

	if (modQueryCount)
	{
		rec.Type = GML::DB::UINT32VAL;
		rec.UInt32Val = nrRecords;
		if (VectPtr.PushByRef(rec)==false)
		{
			notifier->Error("[%s] -> Unable to add record count to vector !",ObjectName);
			return false;
		}
		return true;
	}


	// daca nu mai am linii
	//if (file.ReadNextLine(tempStr)==false)
	//	return false;
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
	//notifier->Info("Reading records: %d (%d)",cIndex,dbPoz);
	// adaug si un hash
	rec.Name = "Hash";
	rec.Type = GML::DB::HASHVAL;
	rec.Hash.Reset();
	
	if (VectPtr.PushByRef(rec)==false)
	{
		notifier->Error("Unable to add HASH to vector !");
		return false;
	}
	//*
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

bool	SimpleTextFileDB::InsertRow (char* Table, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect)
{
	notifier->Error("InsertRow function not suported !");
	return false;
}
bool	SimpleTextFileDB::InsertRow (char* Table, char* Fields, GML::Utils::GTFVector<GML::DB::DBRecord> &Vect)
{
	notifier->Error("InsertRow function not suported !");
	return false;
}
bool	SimpleTextFileDB::Update (char* SqlStatement, GML::Utils::GTFVector<GML::DB::DBRecord> &WhereVals, GML::Utils::GTFVector<GML::DB::DBRecord> &UpdateVals)
{
	notifier->Error("Update function not suported !");
	return false;
}