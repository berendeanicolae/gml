#ifndef __FEATSTATS__
#define __FEATSTATS__

#include "Compat.h"

namespace GML
{
	namespace ML
	{
		struct EXPORT FeatureInformation
		{
			double			countPozitive;
			double			countNegative;
			double			totalPozitive;
			double			totalNegative;
		};
		typedef double (__cdecl *FeatStatComputeFunction)(GML::ML::FeatureInformation *f);
		class EXPORT FeatStatsFunctions
		{
		public:
			static unsigned int 						GetFunctionsCount();
			static char* 								GetFunctionName(unsigned int index);
			static GML::ML::FeatStatComputeFunction 	GetFunctionPointer(unsigned int index);
			
			//Functions
			static double Pozitive(FeatureInformation *f);
			static double TotalPozitive(FeatureInformation *f);
			static double ProcPozitive(FeatureInformation *f);
			static double Negative(FeatureInformation *f);
			static double TotalNegative(FeatureInformation *f);
			static double ProcNegative(FeatureInformation *f);
			static double RapPozNeg(FeatureInformation *f);
			static double ProcDiff(FeatureInformation *f);
			static double ProcAverage(FeatureInformation *f);
			static double ProcTotal(FeatureInformation *f);
			static double AbsProcDiff(FeatureInformation *f);
			static double Diff(FeatureInformation *f);
			static double AbsDiff(FeatureInformation *f);
			static double F1(FeatureInformation *f);
			static double F2(FeatureInformation *f);
			static double ProcTo100(FeatureInformation *f);
			static double AbsProcTo100(FeatureInformation *f);
			static double InformationGain(FeatureInformation *f);
			static double G1(FeatureInformation *f);
			static double G2(FeatureInformation *f);
			static double G3(FeatureInformation *f);
			static double G4(FeatureInformation *f);
			static double GProc(FeatureInformation *f);
			static double GProcTotal(FeatureInformation *f);
			static double ProbPoz(FeatureInformation *f);
			static double ProbNeg(FeatureInformation *f);
			static double MaxProb(FeatureInformation *f);
			static double MedianClosenest(FeatureInformation *f);
			static double AsymetricUncertainty(FeatureInformation *f);
			static double AbsWeightedDiff(FeatureInformation *f);
			static double DiffFromTotal(FeatureInformation *f);
		};
	}
}

#endif