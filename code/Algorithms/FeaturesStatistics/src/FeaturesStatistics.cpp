#include "FeaturesStatistics.h"
#include <math.h>

void ThreadRedirectFunction(GML::Utils::IParalelUnit *paralel,void *context)
{
	FeaturesStatistics *fs = (FeaturesStatistics *)context;
	fs->OnRunThreadCommand(fs->fData[paralel->GetID()],paralel->GetCodeID());
}
//====================================================================================================
int  Compare_FeaturesInformations(FeaturesInformations &f1,FeaturesInformations &f2,void *context)
{
	UInt32	column = *((UInt32 *)context);
	
	if (f1.fnValue[column]>f2.fnValue[column])
		return 1;
	if (f1.fnValue[column]<f2.fnValue[column])
		return -1;
	return 0;
}
int  Compare_FeaturesInformations2(FeaturesInformations &f1,FeaturesInformations &f2)
{
	if (f1.compareValue>f2.compareValue)
		return 1;
	if (f1.compareValue<f2.compareValue)
		return -1;
	return 0;
}
/*
bool FeaturesInformations::operator< (FeaturesInformations &a)
{
	return (bool)(Index<a.Index);
}
bool FeaturesInformations::operator> (FeaturesInformations &a)
{
	return (bool)(Index>a.Index);
}
*/
//====================================================================================================
Stats::Stats()
{
	fnCompute = NULL;
}
Stats::Stats(Stats &ref)
{
	fnCompute = ref.fnCompute;
	this->Name.Set(ref.Name.GetText());
}
FeaturesStatistics::FeaturesStatistics()
{
	GML::Utils::GString		tmp;
	UInt32					tr;
	UInt32					funcCount;

	ObjectName = "FeaturesStatistics";

	SetPropertyMetaData("Command","!!LIST:None=0,Compute!!");

	LinkPropertyToUInt32("ColumnWidth"				,columnWidth			,12,"Sets the column width (0 for no aligniation)");
	LinkPropertyToUInt32("FeatureColumnWidth"		,featureColumnWidth		,20,"Sets the feature name column width (0 for no aligniation)");
	LinkPropertyToString("ResultFile"				,ResultFile				,"","Name of the file to save the result table or none if no save is requared");
	LinkPropertyToBool  ("NotifyResult"				,notifyResults			,true);
	LinkPropertyToBool  ("ShowFeatureName"			,showFeatureName		,true,"Shows feature name in the result list");
	LinkPropertyToDouble("MultiplyFactor"			,multiplyFactor			,1.0);
	LinkPropertyToUInt32("MinPositiveElements"		,MinPoz					,0,"Minimum number of positive elements (for filtering)");
	LinkPropertyToUInt32("MaxPositiveElements"		,MaxPoz					,0xFFFFFFFF,"Maximum number of positive elements (for filtering)");
	LinkPropertyToUInt32("MinNegativeElements"		,MinNeg					,0,"Minimum number of negative elements (for filtering)");
	LinkPropertyToUInt32("MaxNegativeElements"		,MaxNeg					,0xFFFFFFFF,"Maximum number of negative elements (for filtering)");	

	//Add MeasureFunctions from FeatStats from GmlLib
	funcCount = GML::ML::FeatStatsFunctions::GetFunctionsCount();
	for (tr=0;tr<funcCount;tr++)
		AddNewStatFunction(GML::ML::FeatStatsFunctions::GetFunctionName(tr), GML::ML::FeatStatsFunctions::GetFunctionPointer(tr));
	
	SortProps.Set("!!LIST:NoSort=0xFFFF");
	WeightFileType.Set("!!LIST:None=0xFFFF");
	for (tr=0;tr<StatsData.Len();tr++)
	{
		char *text = StatsData[tr].Name.GetText();
		SortProps.AddFormated(",%s=%d",text,tr);
		WeightFileType.AddFormated(",%s=%d",StatsData[tr].Name.GetText(),tr);
	}
	SortProps.Add("!!");
	WeightFileType.Add("!!");

	LinkPropertyToUInt32("SortBy"					,sortBy					,0xFFFF,SortProps.GetText());
	LinkPropertyToUInt32("SortDirection"			,sortDirection			,0,"!!LIST:Ascendent=0,Descendent!!");
	LinkPropertyToUInt32("SaveFeaturesWeight"		,saveFeatureWeightFile	,0xFFFF,WeightFileType.GetText());
	LinkPropertyToString("FeaturesWeightFile"		,FeaturesWeightFile		,"");
}
//====================================================================================================
bool FeaturesStatistics::AddNewStatFunction(char *name,GML::ML::FeatStatComputeFunction _fnCompute)
{
	Stats	tmp;

	if (tmp.Name.Set(name)==false)
		return false;
	tmp.fnCompute = _fnCompute;
	return StatsData.PushByRef(tmp);
}
bool FeaturesStatistics::CreateFeaturesInfo(FeaturesThreadData *fInfo)
{
	UInt32	tr;
	if (con->CreateMlRecord(fInfo->Record)==false)
	{
		notif->Error("[%s] -> Unable to create MLRecord",ObjectName);
		return false;
	}

	if ((fInfo->FI = new FeaturesInfo[con->GetFeatureCount()])==NULL)
	{
		notif->Error("[%s] -> Unable to allocate space for FeaturesInfo (%d)",ObjectName,con->GetFeatureCount());
		return false;
	}
	for (tr=0;tr<con->GetFeatureCount();tr++)
	{
		fInfo->FI[tr].NegativeCount = 0;
		fInfo->FI[tr].PozitiveCount = 0;
	}
	fInfo->totalNegative = 0;
	fInfo->totalPozitive = 0;

	return true;
}
bool FeaturesStatistics::Init()
{
	UInt32		tr,rap;

	if (InitConnections()==false)
		return false;
	// creez obiectele:
	if ((fData = new FeaturesThreadData[threadsCount])==NULL)
	{
		notif->Error("[%s] -> Unable to create %d FeaturesThreadData ",ObjectName,threadsCount);
		return false;
	}
	if (CreateFeaturesInfo(&All)==false)
		return false;
	// pornesc threadPool-ul
	if ((tpu = new GML::Utils::ThreadParalelUnit[threadsCount])==NULL)
	{
		notif->Error("[%s] -> Unable to create %d threads ",ObjectName,threadsCount);
		return false;
	}
	for (tr=0;tr<threadsCount;tr++)
	{
		if (tpu[tr].Init(tr,this,ThreadRedirectFunction)==false)
		{
			notif->Error("[%s] -> Unable to start thread #%d",ObjectName,tr);
			return false;
		}
		if (CreateFeaturesInfo(&fData[tr])==false)
			return false;
	}
	// splituim
	rap = (con->GetRecordCount()/threadsCount)+1;

	// aloc memorie pentru indexi
	for (tr=0;tr<threadsCount;tr++)
	{
		if (tr+1==threadsCount)
			fData[tr].Range.Set(tr*rap,con->GetRecordCount());
		else
			fData[tr].Range.Set(tr*rap,(tr+1)*rap);
	}

	return true;
}
void FeaturesStatistics::OnRunThreadCommand(FeaturesThreadData &ftd,UInt32 command)
{
	UInt32	tr,gr,count,size,index;

	count = con->GetFeatureCount();
	// citesc datele asociate range-ului
	
	size = ftd.Range.End-ftd.Range.Start;
	
	if (ftd.Range.Start==0)
		notif->StartProcent("[%s] -> Computing ... ",ObjectName);	
	for (tr=ftd.Range.Start,index=0;(tr<ftd.Range.End) && (StopAlgorithm==false);tr++,index++)
	{
		if (con->GetRecord(ftd.Record,tr)==false)
		{
			notif->Error("[%s] -> Unable to read record %d",ObjectName,tr);
			return;
		}
		if (ftd.Record.Label==1)
			ftd.totalPozitive++;
		else
			ftd.totalNegative++;
		for (gr=0;(gr<count) && (StopAlgorithm==false);gr++)
			if (ftd.Record.Features[gr]!=0)
            {
				if (ftd.Record.Label==1)
					ftd.FI[gr].PozitiveCount++;
      			else 
					ftd.FI[gr].NegativeCount++;
            }
		if ((ftd.Range.Start==0) && ((index % 1000)==0))
			notif->SetProcent(index,size);
	}
	if (ftd.Range.Start==0)
		notif->EndProcent();	
	
}
bool FeaturesStatistics::CreateHeaders(GML::Utils::GString &str)
{
	UInt32				tr;
	GML::Utils::GString	tmp;

	if (notif->SuportsObjects())
	{
		tmp.Set("Type=List;Column_0=FeatureName;Column_1=Positive;Column_2=Negative;");
		for (tr=0;tr<StatsData.Len();tr++)
			tmp.AddFormated("Column_%d=%s;",tr+3,StatsData[tr].Name.GetText());
		notif->CreateObject("FeatTable",tmp.GetText());
	}
	tmp.Set("");
	if (featureColumnWidth==0)
	{
		if (showFeatureName)
		{
			if (str.Set("FeatName|")==false)
				return false;
		} else {
			if (str.Set("#|")==false)
				return false;
		}
	} else {
		if (showFeatureName)
		{
			if (tmp.Set("FeatName")==false)
				return false;
			while (tmp.Len()<featureColumnWidth)
				if (tmp.AddChar(' ')==false)
					return false;
			tmp.Truncate(featureColumnWidth);
		} else {
			if (tmp.Set("ID   ")==false)
				return false;
		}

		if (str.SetFormated("%s|",tmp.GetText())==false)
			return false;
	}

	for (tr=0;tr<StatsData.Len();tr++)
	{
		if (columnWidth==0)
		{
			if (str.AddFormatedEx("%{str}|",StatsData[tr].Name.GetText())==false)
				return false;
		} else {
			if (str.AddFormatedEx("%{str,L%%,F ,trunc}|",StatsData[tr].Name.GetText(),columnWidth)==false)
				return false;
		}
	}
	return true;
}
bool FeaturesStatistics::CreateRecordInfo(FeaturesInformations &finf,GML::Utils::GString &str)
{
	UInt32				tr;
	GML::Utils::GString	tmp,fName;

	tmp.Set("");
	if (featureColumnWidth==0)
	{
		if (showFeatureName)
		{
			if (con->GetFeatureName(tmp,finf.Index)==false)
				return false;
		} else {
			if (tmp.SetFormated("%d",finf.Index)==false)
				return false;
		}
		if (str.SetFormated("%s|",tmp.GetText())==false)
			return false;
	} else {
		if (showFeatureName)
		{
			if (con->GetFeatureName(tmp,finf.Index)==false)
				return false;
			while (tmp.Len()<featureColumnWidth)
				if (tmp.AddChar(' ')==false)
					return false;
			tmp.Truncate(featureColumnWidth);
		} else {
			if (tmp.SetFormated("%5d",finf.Index)==false)
				return false;
		}
		if (str.SetFormated("%s|",tmp.GetText())==false)
			return false;
	}
	for (tr=0;tr<StatsData.Len();tr++)
	{
		if (tmp.Set("")==false)
			return false;
		if (columnWidth>0)
		{
			if (tr<4)
			{
				if (tmp.AddFormatedEx("%{uint32,R%%,F ,G3}",(UInt32)finf.fnValue[tr],columnWidth)==false)
					return false;
			} else {
				if (tr<6)
				{
					if (tmp.AddFormatedEx("%{double,Z2,R%%,F }%",finf.fnValue[tr],columnWidth-1)==false)
						return false;
				} else {
					if (tmp.AddFormatedEx("%{double,Z4,R%%,F }",finf.fnValue[tr],columnWidth)==false)
						return false;
				}
			}
		} else {
			if (tr<4)
			{
				if (tmp.AddFormatedEx("%{uint32}",(UInt32)finf.fnValue[tr])==false)
					return false;
			} else {
				if (tmp.AddFormatedEx("%{double,Z4,}",finf.fnValue[tr])==false)
					return false;
			}
		}
		if (str.Add(&tmp)==false)
			return false;
		if (str.Add("|")==false)
			return false;
	}
	return true;
}
bool FeaturesStatistics::Validate(FeaturesInformations *fi)
{
	if ((fi->countNegative<MinNeg) || (fi->countNegative>MaxNeg))
		return false;
	if ((fi->countPozitive<MinPoz) || (fi->countPozitive>MaxPoz))
		return false;
	return true;
}
void FeaturesStatistics::PrintStats()
{
	UInt32					tr;
	GML::Utils::GString		str,line;

	if (CreateHeaders(str)==false)
	{
		notif->Error("[%s] -> Unable to create table headers !",ObjectName);
		return;
	}
	line.Set("");
	while (line.Len()<str.Len())
		line.AddChar('-');
	if (notif->SuportsObjects())
	{
		str.Insert("Insert:",0);
		notif->SendDataToObject("FeatTable",str.GetText());
	} else {
		notif->Info(str.GetText());
		notif->Info(line.GetText());
	}
	for (tr=0;tr<con->GetFeatureCount();tr++)
	{
		if (!Validate(&ComputedData[tr]))
			continue;
		if (CreateRecordInfo(ComputedData[tr],str)==false)
		{
			notif->Error("[%s] -> Unable to create record line !",ObjectName);
			return;
		}
		if (notif->SuportsObjects())
		{
			str.Insert("Insert:",0);
			notif->SendDataToObject("FeatTable",str.GetText());
		} else {
			notif->Info(str.GetText());
		}
	}
	if (notif->SuportsObjects()==false)
		notif->Info(line.GetText());
}
void FeaturesStatistics::SaveToFile()
{
	GML::Utils::File	f;

	if (f.Create(ResultFile.GetText())==false)
	{
		notif->Error("[%s] -> Unable to create file : %s",ObjectName,ResultFile.GetText());
		return;
	}
	UInt32					tr;
	GML::Utils::GString		str,line;

	if (CreateHeaders(str)==false)
	{
		notif->Error("[%s] -> Unable to create table headers !",ObjectName);
		return;
	}
	line.Set("");
	while (line.Len()<str.Len())
		line.AddChar('-');
	str.Add("\n");
	line.Add("\n");
	if (f.Write(str.GetText(),str.Len())==false)
	{
		notif->Error("[%s] -> Unable to write to file : %s",ObjectName,ResultFile.GetText());
		return;
	}
	if (f.Write(line.GetText(),str.Len())==false)
	{
		notif->Error("[%s] -> Unable to write to file : %s",ObjectName,ResultFile.GetText());
		return;
	}

	for (tr=0;tr<con->GetFeatureCount();tr++)
	{
		if (!Validate(&ComputedData[tr]))
			continue;
		if (CreateRecordInfo(ComputedData[tr],str)==false)
		{
			notif->Error("[%s] -> Unable to create record line !",ObjectName);
			return;
		}
		str.Add("\n");
		if (f.Write(str.GetText(),str.Len())==false)
		{
			notif->Error("[%s] -> Unable to write to file : %s",ObjectName,ResultFile.GetText());
			return;
		}		
	}
	if (f.Write(line.GetText(),str.Len())==false)
	{
		notif->Error("[%s] -> Unable to write to file : %s",ObjectName,ResultFile.GetText());
		return;
	}
	f.Close();
	notif->Info("[%s] -> %s table saved !",ObjectName,ResultFile.GetText());
}
void FeaturesStatistics::SaveFeatureWeightFile()
{
	GML::Utils::GTFVector<double>	list;
	GML::Utils::AttributeList		attr;
	UInt32							tr;

	// adaug elementele
	for (tr=0;tr<ComputedData.Len();tr++)
	{
		if (list.Push(ComputedData[tr].fnValue[saveFeatureWeightFile])==false)
		{
			notif->Error("[%s] -> Unable to add elements to weight vector in file : %s",ObjectName,FeaturesWeightFile.GetText());
			return;
		}
	}

	if (attr.AddAttribute("Weight",list.GetPtrToObject(0),GML::Utils::AttributeList::DOUBLE,list.Len())==false)
	{
		notif->Error("[%s] -> Unable to create AttributeList in file : %s",ObjectName,FeaturesWeightFile.GetText());
		return;
	}
	if (attr.AddString("Method",StatsData[saveFeatureWeightFile].Name)==false)
	{
		notif->Error("[%s] -> Unable to create AttributeList in file : %s",ObjectName,FeaturesWeightFile.GetText());
		return;
	}
	if (attr.AddDouble("MultiplyFactor",multiplyFactor)==false)
	{
		notif->Error("[%s] -> Unable to create AttributeList in file : %s",ObjectName,FeaturesWeightFile.GetText());
		return;
	}
	if (attr.Save(FeaturesWeightFile.GetText())==false)
	{
		notif->Error("[%s] -> Unable to save file : %s",ObjectName,FeaturesWeightFile.GetText());
		return;
	}
	notif->Info("[%s] -> File (%s) with '%s' features saved",ObjectName,FeaturesWeightFile.GetText(),StatsData[saveFeatureWeightFile].Name);
}
bool FeaturesStatistics::Compute()
{
	UInt32							tr,gr;
	FeaturesInformations			info;
	GML::ML::FeatureInformation		finf;	
	

	notif->Info("[%s] -> Computing statistics ...",ObjectName);
	// executie
	for (tr=0;tr<threadsCount;tr++)
		if (tpu[tr].Execute(1)==false)
		{
			notif->Error("Error on runnig thread #%d",tr);
			return false;
		}
	// asteptare
	for (tr=0;tr<threadsCount;tr++)
		if (tpu[tr].WaitToFinish()==false)
		{
			notif->Error("WaitToFinish failed on thread #%d",tr);
			return false;
		}
	// fac aditia
	for (tr=0;tr<threadsCount;tr++)
	{
		for (gr=0;gr<con->GetFeatureCount();gr++)
		{
			All.FI[gr].PozitiveCount+=fData[tr].FI[gr].PozitiveCount;
			All.FI[gr].NegativeCount+=fData[tr].FI[gr].NegativeCount;
		}
		All.totalNegative+=fData[tr].totalNegative;
		All.totalPozitive+=fData[tr].totalPozitive;
	}
	notif->Info("[%s] -> DataBase processed ok",ObjectName);
	// calculez si functiile
	for (tr=0;tr<con->GetFeatureCount();tr++)
	{
		info.Index = tr;
		info.totalPozitive = finf.totalPozitive = All.totalPozitive; 
		info.totalNegative = finf.totalNegative = All.totalNegative;

		info.countNegative = finf.countNegative = All.FI[tr].NegativeCount;
		info.countPozitive = finf.countPozitive = All.FI[tr].PozitiveCount;
        
		if ((info.fnValue = new double[StatsData.Len()])==NULL)
		{
			notif->Error("[%s] Unable to allocate info.fnValue[%d] !",ObjectName,StatsData.Len());
			return false;
		}
		for (gr=0;gr<StatsData.Len();gr++)
			info.fnValue[gr] = StatsData[gr].fnCompute(&finf) * multiplyFactor;

		if (ComputedData.PushByRef(info)==false)
		{
			notif->Error("[%s] Unable to add informations to vector !",ObjectName);
			return false;
		}
	}
	// verific daca am vreun weight de salvat
	if (saveFeatureWeightFile<StatsData.Len())
	{
		if (FeaturesWeightFile.Len()==0)
		{
			notif->Error("[%s] In order to save '%s' values for a feature you have to complete 'FeaturesWeightFile' property with the name of the file where the features will be saved",ObjectName,StatsData[saveFeatureWeightFile].Name);
		} else {
			SaveFeatureWeightFile();
		}
	}

	// sortez
	if (sortBy!=0xFFFF)
	{
		notif->Info("[%s] -> Sorting data ... ",ObjectName);
		for (UInt32 tr=0;tr<con->GetFeatureCount();tr++)
			ComputedData[tr].compareValue = ComputedData[tr].fnValue[sortBy];
		//ComputedData.Sort(sortDirection==0,Compare_FeaturesInformations,&sortBy);
		ComputedData.Sort(Compare_FeaturesInformations2,sortDirection==0);
	}
	notif->Info("[%s] -> Saving result ... ",ObjectName);
	// printez
	if (notifyResults)
		PrintStats();
	// salvez
	if (ResultFile.Len()>0)
		SaveToFile();
	return true;
}
void FeaturesStatistics::OnExecute()
{
	StopAlgorithm = false;
	
	switch (Command)
	{
		case COMMAND_NONE:
			notif->Info("[%s] -> Nothing to do ... ",ObjectName);
			return;
		case COMMAND_COMPUTE:
			Compute();
			return;
	};
	notif->Error("[%s] -> Unkwnown command ID : %d",ObjectName,Command);
}
