#include "DecisionTreeFeatureStatistics.h"


DecisionTreeFeatureStatistics::DecisionTreeFeatureStatistics()
{
	ObjectName = "DecisionTreeFeatureStatistics";
}
bool DecisionTreeFeatureStatistics::Init()
{
	if (GenericFeatureStatistics::Init()==false)
		return false;
		
	if (BTree.Create(256)==false)
	{
		notif->Error("[%s] -> Unable to create BTree !",ObjectName);
		return false;		
	}
	TreePathSize = 0;
	return true;
}
bool DecisionTreeFeatureStatistics::CreatePath(UInt32 index)
{
	UInt32	*bTree = BTree.GetPtrToObject(0);
	UInt32	remove;
	
	TreePathSize = 0;
	if (index==0)
		return true;
	while (index>0)
	{
		if (TreePathSize>=MAX_PATH_DEPTH)
		{
			notif->Error("[%s] -> Depth to hi (>%d) in BTree for index:%d",ObjectName,MAX_PATH_DEPTH,index);
			return false;
		}
		remove = (((index-1) & 1)<<31);
		index = (index-1)>>1;
		TreePath[TreePathSize++] = bTree[index] | remove;		
	}
	return true;
}
void DecisionTreeFeatureStatistics::OnCompute()
{
	UInt32				tr,gr;
	GML::Utils::File	out;
	
	GML::Utils::GString tmp;
	
	if (out.Create(ResultFileName.GetText(),true)==false)
	{
		notif->Error("[%s] -> Unable to create: %s",ObjectName,ResultFileName.GetText());
		return;
	}
	tmp.Create(2048);
	BTree.DeleteAll();
	workingRecordsCount = con->GetRecordCount();

	while (true)
	{
		// calculez counterele
	
		if (CreatePath(BTree.Len())==false)
			return;
		CreateWorkingList();
		// afisez working list
		tmp.SetFormated("Working List (%d):",TreePathSize);
		for (tr=0;tr<TreePathSize;tr++)
		{
			if (TreePath[tr] & 0x80000000)
				tmp.Add("-");
			else
				tmp.Add("+");
			tmp.AddFormated("%d",TreePath[tr] & 0x7FFFFFFF);
			tmp.Add(",");
		}
		notif->Info("[%s] -> %s",ObjectName,tmp.GetText());

		ComputeScoresAndSort();
		if (FeatScores.Len()==0)
		{
			notif->Info("[%s] -> No more features left ... ending",ObjectName);
			break;
		}
		
		
		featToRemove = FeatScores[0].Index;
		BTree.PushByRef(FeatScores[0].Index);
		
		// afisez
		tmp.SetFormated("%d|Scor:%lf|Poz:%d|Neg:%d|TotalPoz:%d|TotalNeg:%d\n",
						featToRemove,
						FeatScores[0].Score,
						FeatCounters[featToRemove].CountPozitive,
						FeatCounters[featToRemove].CountNegative,
						FeatCounters[featToRemove].CountTotalPozitive,
						FeatCounters[featToRemove].CountTotalNegative);
		
		out.Write(tmp.GetText(),tmp.Len());
	}	
	out.Close();
}
