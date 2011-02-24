#include "RosenblattPerceptron.h"

bool RosenblattPerceptron::DoTrain()
{
	UInt32	it;

	for (it=0;it<maxIterations;it++)
	{
		if (Train(&FullData)==false)
		{
			notif->Error("Error on training ...");
			return false;
		}
		if ((it%testAfterIterations)==0)
		{
			if (Test(&FullData)==false)
			{
				notif->Error("Error on training ...");
				return false;
			}
			notif->Notify(100,&FullData.Res,sizeof(FullData.Res));
		}
	}
	return true;
}
void RosenblattPerceptron::OnExecute(char *command)
{
	if (GML::Utils::GString::Equals(command,"train",true))
	{
		DoTrain();
	}
}