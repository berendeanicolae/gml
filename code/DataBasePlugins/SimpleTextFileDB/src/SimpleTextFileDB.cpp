#include "SimpleTextFileDB.h"

SimpleTextFileDB::SimpleTextFileDB()
{
	FeatNames = NULL;
	ObjectName = "SimpleTextFileDB";
	LinkPropertyToString("FileName",fileName,"","File that contains the database !");
}
bool	SimpleTextFileDB::OnInit()
{
	namedFeaturesCount = 0;
	return true;
}
bool	SimpleTextFileDB::Connect ()
{
	int poz = 0;
	int pzEq;

	namedFeaturesCount = 0;
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
		if (FeatNames[tr].SetFormated("Ft_%d",tr)==false)
			return false;
	}
	namedFeaturesCount = 0;
	for (UInt32 tr=0;tr<nrFeatures;tr++)
	{
		if ((allDB.CopyNextLine(&tempStr,&poz)==true) && (tempStr.Contains("=")))
		{
			tempStr.Strip();
			if ((tempStr.StartsWith("Ft_",true)) || (tempStr.StartsWith("Feat",true)))
			{
				pzEq = tempStr.Find("=");
				tempStr.ReplaceOnPos(0,pzEq,"");
				tempStr.Replace("=","");
				tempStr.Strip();
				if (tempStr.StartsWith("Ft_",false)==false)
				{
					if (FeatNames[tr].SetFormated("Ft_%s",tempStr.GetText())==false)
						return false;
				} else {
					if (FeatNames[tr].SetFormated("%s",tempStr.GetText())==false)
						return false;				
				}
				namedFeaturesCount++;
			} else {
				break;
			}			
		} else {
			break;
		}
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
		// skipez primele 2 + namedFeaturesCount linii
		for (UInt32 tr=0;tr<2+namedFeaturesCount;tr++)
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
	rec.Type = GML::DB::TYPES::HASH;
	
	if (VectPtr.PushByRef(rec)==false)
	{
		notifier->Error("Unable to add HASH to vector !");
		return false;
	}
	rec.Name = "Label";
	rec.Type = GML::DB::TYPES::DOUBLE;
	if (VectPtr.PushByRef(rec)==false)
	{
		notifier->Error("Unable to add label to vector !");
		return false;
	}
	rec.Type = GML::DB::TYPES::DOUBLE;	
	for (UInt32 tr=0;tr<nrFeatures;tr++)
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
	char				hash[33];

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
	
	// formatul este [hash]:label:lista flaguri
	tempStr.Strip();
	tempStr.Replace(" ","");
	tempStr.Replace("\t","");

	cIndex++;
	//notifier->Info("Reading records: %d (%d)",cIndex,dbPoz);
	// adaug si un hash
	rec.Name = "Hash";
	rec.Type = GML::DB::TYPES::HASH;
	rec.Value.Hash.Reset();
	if (tempStr.StartsWith("[]:"))
	{
		tempStr.ReplaceOnPos(0,4,"");
		tempStr.Strip();
	}
	if (tempStr.StartsWith("["))
	{
		// am si hash in lista
		int poz = tempStr.Find("]");
		if (poz!=33)
		{
			notifier->Error("[%s] -> Invalid line format : %s\n",ObjectName,tempStr.GetText());
			return false;
		}
		memcpy(hash,&tempStr.GetText()[1],32);
		hash[32]=0;
		if (rec.Value.Hash.CreateFromText(hash)==false)
		{
			notifier->Error("[%s] -> Invalid line format : %s (invalid hash value)\n",ObjectName,tempStr.GetText());
			return false;
		}
		tempStr.ReplaceOnPos(0,35,"");
		tempStr.Strip();
	}
	
	if (VectPtr.PushByRef(rec)==false)
	{
		notifier->Error("[%s] -> Unable to add HASH to vector !",ObjectName);
		return false;
	}
	//*

	rec.Name = "Label";
	rec.Type = GML::DB::TYPES::DOUBLE;
	if (tempStr.StartsWith("1:"))
	{
		rec.Value.DoubleVal = 1.0;
		tempStr.ReplaceOnPos(0,2,"");
	} else {
		if (tempStr.StartsWith("-1:"))
		{
			rec.Value.DoubleVal = -1.0;
			tempStr.ReplaceOnPos(0,3,"");
		} else {
			notifier->Error("[%s] -> Invalid format : %s",ObjectName,tempStr.GetText());
			return false;
		}
	}
	if (VectPtr.PushByRef(rec)==false)
	{
		notifier->Error("[%s] -> Unable to add label to vector !",ObjectName);
		return false;
	}
	// urc toate featurerile
	rec.Type = GML::DB::TYPES::DOUBLE;
	rec.Value.DoubleVal = 0;
	for (UInt32 tr=0;tr<nrFeatures;tr++)
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
			notifier->Error("[%s] -> Invalid value for feature index (%s) => %s",ObjectName,featureIndex.GetText(),tempStr.GetText());
			return false;
		}
		if (index>=nrFeatures)
		{
			notifier->Error("[%s] -> Index outside range[0..%d] (%d) => %s",ObjectName,nrFeatures,index,tempStr.GetText());
			return false;
		}
		// VectPtr[0] = Hash
		// VectPtr[1] = Label
		// VectPtr[2] = Feature[0]
		VectPtr[index+2].Value.DoubleVal = 1.0;
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