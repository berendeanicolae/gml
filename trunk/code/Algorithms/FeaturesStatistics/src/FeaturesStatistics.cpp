#include "FeaturesStatistics.h"

int  Compare_FeaturesInformations(FeaturesInformations &f1,FeaturesInformations &f2)
{
	if (f1.compareValue>f2.compareValue)
		return 1;
	if (f1.compareValue<f2.compareValue)
		return -1;
	return 0;
}
//==============================================================================
FeaturesStatistics::FeaturesStatistics()
{
	GML::Utils::GString		tmp;
	UInt32					tr;
		
	ObjectName = "FeaturesStatistics";

	//Add extra commands here
	SetPropertyMetaData("Command","!!LIST:None=0,Compute!!");
	
	LinkPropertyToUInt32("ColumnWidth"				,columnWidth			,12,"Sets the column width (0 for no aligniation)");
	LinkPropertyToUInt32("FeatureColumnWidth"		,featureColumnWidth		,20,"Sets the feature name column width (0 for no aligniation)");
	LinkPropertyToString("ResultFile"				,ResultFile				,"","Name of the file to save the result table or none if no save is requared");
	LinkPropertyToBool  ("NotifyResult"				,notifyResults			,true);
	LinkPropertyToBool  ("ShowFeatureName"			,showFeatureName		,true,"Shows feature name in the result list");
	LinkPropertyToBool  ("AdjustToNumberOfFeatures"	,AdjustToNumberOfFeatures,false,"Adjust each feature to the number of features that are set in each record");
	LinkPropertyToDouble("MultiplyFactor"			,multiplyFactor			,1.0);
	LinkPropertyToUInt32("MinPositiveElements"		,MinPoz					,0,"Minimum number of positive elements (for filtering)");
	LinkPropertyToUInt32("MaxPositiveElements"		,MaxPoz					,0xFFFFFFFF,"Maximum number of positive elements (for filtering)");
	LinkPropertyToUInt32("MinNegativeElements"		,MinNeg					,0,"Minimum number of negative elements (for filtering)");
	LinkPropertyToUInt32("MaxNegativeElements"		,MaxNeg					,0xFFFFFFFF,"Maximum number of negative elements (for filtering)");	

	//Add MeasureFunctions from FeatStats from GmlLib
	statFuncCount = GML::ML::FeatStatsFunctions::GetFunctionsCount();
	SortProps.Set("!!LIST:NoSort=0xFFFF");
	WeightFileType.Set("!!LIST:None=0xFFFF");
	for (tr=0;tr<statFuncCount;tr++)
	{
		char *text = GML::ML::FeatStatsFunctions::GetFunctionName(tr);
		SortProps.AddFormated(",%s=%d",text,tr);
		WeightFileType.AddFormated(",%s=%d",text,tr);
	}
	SortProps.Add("!!");
	WeightFileType.Add("!!");

	LinkPropertyToUInt32("SortBy"					,sortBy					,0xFFFF,SortProps.GetText());
	LinkPropertyToUInt32("SortDirection"			,sortDirection			,0,"!!LIST:Ascendent=0,Descendent!!");
	LinkPropertyToUInt32("SaveFeaturesWeight"		,saveFeatureWeightFile	,0xFFFF,WeightFileType.GetText());
	LinkPropertyToString("FeaturesWeightFile"		,FeaturesWeightFile		,"");		
}
bool FeaturesStatistics::Init()
{
	if (InitConnections()==false)
		return false;
	if (InitThreads()==false)
		return false;
	if (SplitMLThreadDataRange(con->GetRecordCount())==false)
		return false;
	if (con->CreateMlRecord(MainRecord)==false)
	{
		notif->Error("[%s] -> Unable to create MainRecord",ObjectName);
		return false;
	}
	if (ComputedData.Create(con->GetFeatureCount(),true)==false)
	{
		notif->Error("[%s] -> Unable to create ComputedData",ObjectName);
		return false;	
	}
	if (Feats.Create(con->GetFeatureCount(),true)==false)
	{
		notif->Error("[%s] -> Unable to create Feats Global information",ObjectName);
		return false;	
	}
	for (UInt32 tr=0;tr<con->GetFeatureCount();tr++)
	{
		if ((ComputedData[tr].fnValue = new double[statFuncCount])==NULL)
		{
			notif->Error("[%s] -> Unable to alloc data for CompuedData Values",ObjectName);
			return false;		
		}
	}
	return true;
}
void FeaturesStatistics::OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand)
{
	switch (threadCommand)
	{
		case THREAD_COMMAND_NONE:
			// do nothing
			return;
		case THREAD_COMMAND_COMPUTE:
			OnComputeFeatureCounters(thData);
			return;
	};
}
bool FeaturesStatistics::OnComputeFeatureCounters(GML::Algorithm::MLThreadData &thData)
{
	UInt32							tr,gr,nrRecords,nrFeatures;
	FeaturesStatisticsThreadData	*obj_td = (FeaturesStatisticsThreadData* )thData.Context;
	double							featCountInRec;
	
	nrRecords = con->GetRecordCount();
	nrFeatures = con->GetFeatureCount();
	// curatam datele
	MEMSET(obj_td->Feats,0,sizeof(GML::ML::FeatureInformation)*nrFeatures);
	
	if (thData.ThreadID==0)
		notif->StartProcent("[%s] -> Analyzing (%d record) ",ObjectName,nrRecords);
		
	for (tr=thData.ThreadID;tr<nrRecords;tr+=threadsCount)
	{		
		if (con->GetRecord(thData.Record,tr)==false)
		{
			notif->Error("[%s] -> Unable to read record #%d",ObjectName,tr);
			return false;
		}
		if (AdjustToNumberOfFeatures)
		{
			featCountInRec = 0;
			for (gr=0;gr<nrFeatures;gr++)
				if (thData.Record.Features[gr]!=0)
					featCountInRec++;
			featCountInRec = 1/featCountInRec;
			for (gr=0;gr<nrFeatures;gr++)
			{
				if (thData.Record.Features[gr]!=0)
				{
					if (thData.Record.Label>0)
					{
						obj_td->Feats[gr].countPozitive+=featCountInRec;
						obj_td->Feats[gr].totalPozitive+=featCountInRec;
					} else {
						obj_td->Feats[gr].countNegative+=featCountInRec;
						obj_td->Feats[gr].totalNegative+=featCountInRec;
					}
				} else {
					if (thData.Record.Label>0)
						obj_td->Feats[gr].totalPozitive+=featCountInRec;
					else
						obj_td->Feats[gr].totalNegative+=featCountInRec;
				}
			}				
		} else {		
			for (gr=0;gr<nrFeatures;gr++)
			{
				if (thData.Record.Features[gr]!=0)
				{
					if (thData.Record.Label>0)
					{
						obj_td->Feats[gr].countPozitive++;
						obj_td->Feats[gr].totalPozitive++;
					} else {
						obj_td->Feats[gr].countNegative++;
						obj_td->Feats[gr].totalNegative++;
					}
				} else {
					if (thData.Record.Label>0)
						obj_td->Feats[gr].totalPozitive++;
					else
						obj_td->Feats[gr].totalNegative++;
				}
			}
		}
		if ((thData.ThreadID==0) && ((tr%10000)==0))
			notif->SetProcent(tr,nrRecords);
	}
	if (thData.ThreadID==0)
		notif->EndProcent();
	
	return true;
}
void FeaturesStatistics::Compute()
{
	UInt32							tr,gr;
	
	ExecuteParalelCommand(THREAD_COMMAND_COMPUTE);	
	MEMSET(Feats.GetPtrToObject(0),0,sizeof(GML::ML::FeatureInformation)*con->GetFeatureCount());
	
	for (tr=0;tr<threadsCount;tr++)
	{		
		for (gr=0;gr<con->GetFeatureCount();gr++)
		{
			Feats[gr].countPozitive += ((FeaturesStatisticsThreadData *)ThData[tr].Context)->Feats[gr].countPozitive;
			Feats[gr].totalPozitive += ((FeaturesStatisticsThreadData *)ThData[tr].Context)->Feats[gr].totalPozitive;
			Feats[gr].countNegative += ((FeaturesStatisticsThreadData *)ThData[tr].Context)->Feats[gr].countNegative;
			Feats[gr].totalNegative += ((FeaturesStatisticsThreadData *)ThData[tr].Context)->Feats[gr].totalNegative;
		}
	}
	notif->Info("[%s] -> DataBase processed ok",ObjectName);
	for (tr=0;tr<con->GetFeatureCount();tr++)
	{
		ComputedData[tr].Index = tr;
		for (gr=0;gr<statFuncCount;gr++)
			ComputedData[tr].fnValue[gr] = GML::ML::FeatStatsFunctions::GetFunctionPointer(gr)(Feats.GetPtrToObject(tr)) * multiplyFactor;
		if (sortBy<0xFFFF)
			ComputedData[tr].compareValue = ComputedData[tr].fnValue[sortBy];
	}
	// am calculat si valorile
	// verific daca am vreun weight de salvat
	if (saveFeatureWeightFile<statFuncCount)
	{
		if (FeaturesWeightFile.Len()==0)
		{
			notif->Error("[%s] In order to save '%s' values for a feature you have to complete 'FeaturesWeightFile' property with the name of the file where the features will be saved",
					ObjectName,
					GML::ML::FeatStatsFunctions::GetFunctionName(saveFeatureWeightFile));
		} else {
			SaveFeatureWeightFile();
		}
	}
	// sortez
	if (sortBy!=0xFFFF)
	{
		notif->Info("[%s] -> Sorting data ... ",ObjectName);
		ComputedData.Sort(Compare_FeaturesInformations,sortDirection==0);
	}
	notif->Info("[%s] -> Saving result ... ",ObjectName);
	// printez
	if (notifyResults)
		PrintStats();
	// salvez
	if (ResultFile.Len()>0)
		SaveToFile();	
}
bool FeaturesStatistics::OnInitThreadData(GML::Algorithm::MLThreadData &thData)
{
	FeaturesStatisticsThreadData	*obj_td = new FeaturesStatisticsThreadData();
	if (obj_td==NULL)
		return false;
	if ((obj_td->Feats = new GML::ML::FeatureInformation[con->GetFeatureCount()])==NULL)
		return false;
	thData.Context = obj_td;
	return true;
}
bool FeaturesStatistics::CreateHeaders(GML::Utils::GString &str)
{
	UInt32				tr;
	GML::Utils::GString	tmp;

	if (notif->SuportsObjects())
	{
		tmp.Set("Type=List;Column_0=FeatureName;Column_1=Positive;Column_2=Negative;");
		for (tr=0;tr<statFuncCount;tr++)
			tmp.AddFormated("Column_%d=%s;",tr+3,GML::ML::FeatStatsFunctions::GetFunctionName(tr));
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

	for (tr=0;tr<statFuncCount;tr++)
	{
		if (columnWidth==0)
		{
			if (str.AddFormatedEx("%{str}|",GML::ML::FeatStatsFunctions::GetFunctionName(tr))==false)
				return false;
		} else {
			if (str.AddFormatedEx("%{str,L%%,F ,trunc}|",GML::ML::FeatStatsFunctions::GetFunctionName(tr),columnWidth)==false)
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
	for (tr=0;tr<statFuncCount;tr++)
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

bool FeaturesStatistics::Validate(GML::ML::FeatureInformation *fi)
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
		if (!Validate(&Feats[ComputedData[tr].Index]))
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
		if (!Validate(&Feats[ComputedData[tr].Index]))
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
	if (attr.AddString("Method",GML::ML::FeatStatsFunctions::GetFunctionName(saveFeatureWeightFile))==false)
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
	notif->Info("[%s] -> File (%s) with '%s' features saved",ObjectName,FeaturesWeightFile.GetText(),GML::ML::FeatStatsFunctions::GetFunctionName(saveFeatureWeightFile));
}
void FeaturesStatistics::OnExecute()
{
	switch (Command)
	{
		case COMMAND_NONE:
			notif->Error("[%s] -> Nothing to do , select another command ",ObjectName);
			break;
		case COMMAND_COMPUTE:
			Compute();
			break;
		default:
			notif->Error("[%s] -> Unknown command ID: %d",ObjectName,Command);
			break;
	}	
}
