#include "Projectron.h"

Projectron::Projectron()
{
	ObjectName = "Projectron";

	db = NULL;
	con = NULL;

	LinkPropertyToString("Connector",strConector,"","Conectorul la care sa se conecteze");
	LinkPropertyToString("Notifier",strNotificator,"","Notificatorul la care sa se conecteze");
	LinkPropertyToString("DataBase",strDB,"","Baza de date la care sa se conecteze");
	LinkPropertyToUInt32("KernelFunction",kernelFunction,POLYNOMIAL_KER,"!!LIST:Polinomiala=0,Sigmoida!!\nMetrica folosita");

	SetPropertyMetaData("Command","!!LIST:None=0,Train,Test!!");
}

bool Projectron::Init()
{
	if ((notif = GML::Builder::CreateNotifier(strNotificator.GetText()))==NULL)
		return false;
	if ((db = GML::Builder::CreateDataBase(strDB.GetText(),*notif))==NULL)
	{
		notif->Error("[%s] -> Unable to create Database (%s)",strDB.GetText());
		return false;
	}
	if (db->Connect()==false)
	{
		notif->Error("[%s] -> Unable to connesct to Database (%s)",ObjectName,strDB.GetText());
		return false;
	}
	if ((con = GML::Builder::CreateConnectors(strConector.GetText(),*notif,*db))==NULL)
	{
		notif->Error("[%s] -> Unable to create Conector (%s)",ObjectName,strConector.GetText());
		return false;
	}

	notif->Info("Init: %d");
	return true;
}

bool Projectron::PerformTrain()
{
	return true;
}
bool Projectron::PerformTest()
{
	return true;
}

void Projectron::OnExecute()
{
	switch (Command)
	{

	case COMMAND_TRAIN:
		if(!PerformTrain()){
			notif->Error("Error encountered on training!");
			return;
		}
		notif->Info("Training sequence performed succesfully.");
		return;
	case COMMAND_TEST:
		if(!PerformTest()){
			notif->Error("Error encountered on testing!!");
		}
		notif->Info("Testing sequence performed succesfully.");
		return;
	case COMMAND_NONE:
			notif->Info("[%s] -> Nothing to do ... ",ObjectName);
			return;
	}
}