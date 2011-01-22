#include "StdAfx.h"
#include "Timer.h"

UInt32 GetTimeInMiliSeconds()
{
#ifdef OS_WINDOWS
	return GetTickCount();
#endif
	return 0;
}
void TranslateToTime(UInt32 Diff,GML::Utils::GString *text)
{
	UInt32 h,m,s,ms;
	// 1 min = 60 sec = 60000 ms
	// 1 hour = 3600 sec = 3600000 ms
	
	h = Diff / 3600000;
	Diff %= 3600000;
	m = Diff / 60000;
	Diff %= 60000;
	s = Diff / 1000;
	ms = Diff % 1000;

	text->SetFormated("%4d:%02d:%02d.%03d",h,m,s,ms);
}
//===============================================
void GML::Utils::Timer::Start()
{
	timeStart = GetTimeInMiliSeconds();
	timeDiff = 0;
}
void GML::Utils::Timer::Stop()
{
	UInt32	End;

	End = GetTimeInMiliSeconds();
	// partea de diferente
	if (End>=timeStart)
		timeDiff = End-timeStart;
	else
		timeDiff = (0xFFFFFFFF-timeStart)+End;

	TranslateToTime(timeDiff,&Result);
}
UInt32	GML::Utils::Timer::GetPeriodAsMiliSeconds()
{
	return timeDiff;
}
char*	GML::Utils::Timer::GetPeriodAsString()
{
	return Result.GetText();
}
char* GML::Utils::Timer::EstimateTotalTime(UInt32 parts,UInt32 total)
{
	UInt32 new_diff;

	if (parts==0)
		return "";
	if (parts>total)
		parts = total;
	new_diff = (UInt32)( ((double)total) * ((double)timeDiff) / ((double)parts) );
	TranslateToTime(new_diff,&Result);
	return Result.GetText();
}
char* GML::Utils::Timer::EstimateETA(UInt32 parts,UInt32 total)
{
	UInt32 new_diff;

	if (parts==0)
		return "";
	if (parts>total)
		parts = total;
	new_diff = (UInt32)(((double)(total-parts)) * ((double)timeDiff) / ((double)parts) );
	TranslateToTime(new_diff,&Result);
	return Result.GetText();
}

