#include "FeatureValuesStatistics.h"

int FeatValueCompare(ValueInfos &v1,ValueInfos &v2)
{
	if (v1.Value<v2.Value)
		return -1;
	if (v1.Value>v2.Value)
		return 1;
	if (v1.Label<v2.Label)
		return -1;
	if (v1.Label>v2.Label)
		return 1;
	return 0;
}
int doubleCompare(double &v1,double &v2)
{
	if (v1>v2)
		return 1;
	if (v1<v2)
		return -1;
	return 0;
}
//==============================================================================================
FeatureValuesStatistics::FeatureValuesStatistics()
{
	ObjectName = "FeatureValuesStatistics";

	//Add extra commands here
	SetPropertyMetaData("Command","!!LIST:None=0,Compute!!");

	LinkPropertyToString("ResultFile",ResultFile,"","Output result file ...");
}
bool FeatureValuesStatistics::InitLabels()
{
	double Label;
	
	if (Labels.Create(512)==false)
	{
		notif->Error("[%s] -> Unable to create value array",ObjectName);
		return false;	
	}
	// adaug clasele
	for (UInt32 tr=0;tr<con->GetRecordCount();tr++)
	{
		if (con->GetRecordLabel(Label,tr)==false)
		{
			notif->Error("[%s] -> Unable to read label for record #%d",ObjectName,tr);
			return false;			
		}
		if (Labels.BinarySearch(Label,doubleCompare)<0)
		{
			if (Labels.Insert(Label,doubleCompare)==false)
			{
				notif->Error("[%s] -> Unable to add label to list",ObjectName);
				return false;			
			}
		}
	}
	notif->Info("[%s] -> Total labels: %d",ObjectName,Labels.Len());
	return true;
}
bool FeatureValuesStatistics::Init()
{
	if (InitConnections()==false)
		return false;
	if (InitLabels()==false)
		return false;
	if (InitThreads()==false)
		return false;
	if (SplitMLThreadDataRange(con->GetRecordCount())==false)
		return false;
	if (Vi.Create(con->GetRecordCount(),true)==false)
	{
		notif->Error("[%s] -> Unable to create value array",ObjectName);
		return false;	
	}
	if (con->CreateMlRecord(MainRecord)==false)
	{
		notif->Error("[%s] -> Unable to create MainRecord",ObjectName);
		return false;
	}
	// Add extra initialization here
	return true;
}
bool FeatureValuesStatistics::ComputeValues(GML::Algorithm::MLThreadData &thData)
{
	UInt32	nrRecords = con->GetRecordCount();
	UInt32	featIndex = ((FeatureValuesStatisticsThreadData *)thData.Context)->FeatIndex;
	int		l_index;
	
	for (UInt32 tr=thData.ThreadID;(tr<nrRecords) && (StopAlgorithm==false);tr+=threadsCount)
	{
		if (con->GetRecord(thData.Record,tr)==false)
		{
			notif->Error("[%s] -> Unable to read record %d",ObjectName,tr);
			return false;
		}
		Vi[tr].Label = thData.Record.Label;
		Vi[tr].Value = thData.Record.Features[featIndex];
	}
	return true;
}
bool FeatureValuesStatistics::ComputeValuesForFeature(UInt32 index,GML::Utils::File &f)
{
	GML::Utils::GString		temp,data;
	UInt32					tr,start,count;
	double					value,label;
	int						lastLabel,cLabel;
	
	for (tr=0;tr<threadsCount;tr++)
		((FeatureValuesStatisticsThreadData *)ThData[tr].Context)->FeatIndex=index;
	// rulez in paralel	
	ExecuteParalelCommand(THREAD_COMMAND_COMPUTE_VALUES);
	// sortez datele	
	Vi.Sort(FeatValueCompare,true);
	// scriu datele	
	if (con->GetFeatureName(temp,index)==false)
	{
		notif->Error("[%s] -> Unable to read feature name %d",ObjectName,index);
		return false;
	}	
	// scriu datele
	if (data.SetFormated("Feature (%d) => %s\n",index,temp.GetText())==false)
	{
		notif->Error("[%s] -> Internal error (unable to create information string)",ObjectName);
		return false;	
	}
	// value
	if (data.Add("\t|Value          |")==false)
	{
		notif->Error("[%s] -> Internal error (unable to create information string) - (6)",ObjectName);
		return false;	
	}
	for (tr=0;tr<Labels.Len();tr++)
	{
		if (data.AddFormatedEx("C:%{double,Z3,L10,Trunc}|",Labels[tr])==false)
		{
			notif->Error("[%s] -> Internal error (unable to create information string) - (7)",ObjectName);
			return false;	
		}
	}
	if (data.Add("Total       |\n\t")==false)
	{
		notif->Error("[%s] -> Internal error (unable to create information string) - (8)",ObjectName);
		return false;	
	}
	if (data.AddChars('-',17+13*(Labels.Len()+1))==false)
	{
		notif->Error("[%s] -> Internal error (unable to create information string) - (8)",ObjectName);
		return false;	
	}	
	if (data.AddFormatedEx("\n\t|%{double,Z3,L15}|",Vi[0].Value)==false)
	{
		notif->Error("[%s] -> Internal error (unable to create information string) - (2)",ObjectName);
		return false;			
	}	
	lastLabel = 0;
	for (tr=start=0,count=0,value=Vi[0].Value,label=Vi[0].Label;tr<Vi.Len();tr++,count++)
	{
		if (Vi[tr].Value!=value)
		{
			cLabel = Labels.BinarySearch(label,doubleCompare);
			while(lastLabel<cLabel)
			{
				data.Add("            |");
				lastLabel++;
			}
			if (data.AddFormatedEx("%{uint32,G3,R12}|",count)==false)
			{
				notif->Error("[%s] -> Internal error (unable to create information string) - (4)",ObjectName);
				return false;			
			}
			lastLabel++;
			while(lastLabel<Labels.Len())
			{
				data.Add("            |");
				lastLabel++;
			}			
			if (data.AddFormatedEx("%{uint32,G3,R12}|\n",tr-start)==false)
			{
				notif->Error("[%s] -> Internal error (unable to create information string) - (2)",ObjectName);
				return false;			
			}
			if (f.Write(data.GetText(),data.Len())==false)
			{
				notif->Error("[%s] -> Unable to write data to file ...",ObjectName);
				return false;			
			}
			data.Set("");
			data.Truncate(0);
			if (data.AddFormatedEx("\t|%{double,Z3,L15}|",Vi[tr].Value)==false)			
			{
				notif->Error("[%s] -> Internal error (unable to create information string) - (3)",ObjectName);
				return false;			
			}
			count = 0;
			start = tr;
			value = Vi[tr].Value;
			label = Vi[tr].Label;			
			lastLabel = 0;
			continue;
		}
		if (Vi[tr].Label!=label)
		{
			cLabel = Labels.BinarySearch(label,doubleCompare);
			while(lastLabel<cLabel)
			{
				data.Add("            |");
				lastLabel++;
			}
			if (data.AddFormatedEx("%{uint32,G3,R12}|",count)==false)
			
			{
				notif->Error("[%s] -> Internal error (unable to create information string) - (4)",ObjectName);
				return false;			
			}
			count=0;
			label = Vi[tr].Label;
			lastLabel++;
			continue;
		}		
	}
	cLabel = Labels.BinarySearch(label,doubleCompare);
	while(lastLabel<cLabel)
	{
		data.Add("            |");
		lastLabel++;
	}
	if (data.AddFormatedEx("%{uint32,G3,R12}|",count)==false)
	{
		notif->Error("[%s] -> Internal error (unable to create information string) - (4)",ObjectName);
		return false;			
	}
	lastLabel++;
	while(lastLabel<Labels.Len())
	{
		data.Add("            |");
		lastLabel++;
	}			
	if (data.AddFormatedEx("%{uint32,G3,R12}|\n\n",tr-start)==false)
	{
		notif->Error("[%s] -> Internal error (unable to create information string) - (2)",ObjectName);
		return false;			
	}	
	if (f.Write(data.GetText(),data.Len())==false)
	{
		notif->Error("[%s] -> Unable to write data to file ...",ObjectName);
		return false;			
	}
	return true;
}
void FeatureValuesStatistics::OnRunThreadCommand(GML::Algorithm::MLThreadData &thData,UInt32 threadCommand)
{
	switch (threadCommand)
	{
		case THREAD_COMMAND_COMPUTE_VALUES:
			ComputeValues(thData);
			return;		
	};
}
bool FeatureValuesStatistics::OnInitThreadData(GML::Algorithm::MLThreadData &thData)
{
	FeatureValuesStatisticsThreadData	*obj_td = new FeatureValuesStatisticsThreadData();
	if (obj_td==NULL)
		return false;
	thData.Context = obj_td;
	return true;
}
bool FeatureValuesStatistics::Compute()
{
	GML::Utils::File	f;
	
	if (f.Create(ResultFile.GetText())==false)
	{
		notif->Error("[%s] -> Unable to create file: %s",ObjectName,ResultFile.GetText());
		return false;
	}
	notif->StartProcent("[%s] -> Computing ... ",ObjectName);
	for (UInt32 tr=0;tr<con->GetFeatureCount();tr++)
	{
		if (ComputeValuesForFeature(tr,f)==false)
			return false;
		notif->SetProcent(tr,con->GetFeatureCount());
	}
	notif->EndProcent();
	f.Close();
	notif->Info("[%s] -> %s saved ok !",ObjectName,ResultFile.GetText());
	return true;
}
void FeatureValuesStatistics::OnExecute()
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
