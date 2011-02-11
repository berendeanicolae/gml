#include "SimpleTextFileDB.h"

bool	SimpleTextFileDB::OnInit()
{
	if (Attr.UpdateString("FileName",fileName)==false)
	{
		notifier->Error("Missing 'FileName' attribute");
		return false;
	}
	FeatNames = NULL;
	return true;
}
bool	SimpleTextFileDB::Connect ()
{
	if (file.OpenRead(fileName.GetText())==false)
	{
		notifier->Error("Unable to open (%s) for reading !",fileName.GetText());
		return false;
	}
	// citesc prima linie
	if (file.ReadNextLine(tempStr)==false)
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
	// citesc a doua linie
	if (file.ReadNextLine(tempStr)==false)
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
	// all ok
	if ((FeatNames = new GML::Utils::GString[nrFeatures])==NULL)
		return false;
	for (UInt32 tr = 0;tr<nrFeatures;tr++)
	{
		if (FeatNames[tr].SetFormated("Feat_%d",tr)==false)
			return false;
	}
	return true;
}
bool	SimpleTextFileDB::Disconnect ()
{
	file.Close();
	if (FeatNames!=NULL)
		delete []FeatNames;
	FeatNames = NULL;
	return true;
}
UInt32	SimpleTextFileDB::Select (char* Statement)
{
	if ((GML::Utils::GString::Equals(Statement,"*")==false) &&
		(GML::Utils::GString::Equals(Statement,"select * from RecordsTable")==false))
	{
		notifier->Error("Only 'Select(*)' is suported !");
		return 0;
	}
	if (file.SetFilePos(0)==false)
	{
		notifier->Error("File::Seek(0) error !!!");
		return 0;
	}
	// skipez primele 2 linii
	for (int tr=0;tr<2;tr++)
	{
		if (file.ReadNextLine(tempStr)==false)
		{
			notifier->Error("File::Error reading a line from %s",fileName.GetText());
			return 0;
		}
	}
	cIndex = 0;
	// totul e ok
	return nrRecords;
}
UInt32	SimpleTextFileDB::SqlSelect (char* What, char* Where, char* From)
{
	notifier->Error("SqlSelect function not suported !");
	return 0;
}
bool	SimpleTextFileDB::FetchNextRow (GML::Utils::GTVector<GML::DB::DBRecord> &VectPtr)
{
	GML::DB::DBRecord	rec;
	int					poz;
	UInt32				index;

	if (VectPtr.DeleteAll()==false)
	{
		notifier->Error("Unable to delete all indexes from record vector");
		return false;
	}
	// daca nu mai am linii
	if (file.ReadNextLine(tempStr)==false)
		return false;

	cIndex++;
	// adaug si un hash
	rec.Name = "HASH";
	rec.Type = GML::DB::HASHVAL;
	memset(rec.Hash.Value,0,sizeof(rec.Hash));
	if (VectPtr.PushByRef(rec)==false)
	{
		notifier->Error("Unable to add HASH to vector !");
		return false;
	}

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
		// VectPtr[0] = ID
		// VectPtr[1] = Hash
		// VectPtr[2] = Label
		// VectPtr[3] = Feature[0]
		VectPtr[index+3].DoubleVal = 1.0;
	}
	return true;
}
bool	SimpleTextFileDB::FetchRowNr (GML::Utils::GTVector<GML::DB::DBRecord> &VectPtr, UInt32 RowNr)
{
	notifier->Error("FetchRowNr function not suported !");
	return false;
}
bool	SimpleTextFileDB::FreeRow(GML::Utils::GTVector<GML::DB::DBRecord> &Vect)
{
	// nu am facut nici o alocare -> deci nu trebuie sa eliberez ceva
	return true;
}
bool	SimpleTextFileDB::InsertRow (char* Table, GML::Utils::GTVector<GML::DB::DBRecord> &Vect)
{
	notifier->Error("InsertRow function not suported !");
	return false;
}
bool	SimpleTextFileDB::InsertRow (char* Table, char* Fields, GML::Utils::GTVector<GML::DB::DBRecord> &Vect)
{
	notifier->Error("InsertRow function not suported !");
	return false;
}
bool	SimpleTextFileDB::Update (char* SqlStatement, GML::Utils::GTVector<GML::DB::DBRecord> &WhereVals, GML::Utils::GTVector<GML::DB::DBRecord> &UpdateVals)
{
	notifier->Error("Update function not suported !");
	return false;
}