#include "CoarseGrainedPattern.h"



CoarseGrainedPattern::CoarseGrainedPattern():patternClusters(list<PatternCluster*>())
{
}

void CoarseGrainedPattern::outputCGP()
{
	//srand(unsigned(time(NULL)));
	int startType = patternClusters.front()->semanticType;
	int endType = patternClusters.back()->semanticType;
	int randNum = rand() % 1000;
	stringstream ss;
	ss << startType << "_" << endType << "_" << randNum << ".txt";
	ofstream fout(ss.str());
	for each (PatternCluster* patternClusterPtr in patternClusters)
	{
		for each (auto edgePtr in patternClusterPtr->edges)
		{
			fout << edgePtr->id << endl;
		}
		fout <<"-1"<< endl;
	}
	fout.close();
}


CoarseGrainedPattern::~CoarseGrainedPattern()
{
}
