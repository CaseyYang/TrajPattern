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
	set<int> edges = set<int>();
	for each (PatternCluster* patternClusterPtr in patternClusters)
	{
		for each (FineGrainedPattern* patternPtr in patternClusterPtr->patterns)
		{
			for each (auto edgeClusterPtr in patternPtr->edgeClusterPattern)
			{
				edges.insert(edgeClusterPtr->clusterCoreEdge->id);
			}
		}
		for each (auto edgeIndex in edges)
		{
			fout << edgeIndex << endl;
		}
		fout <<"-1"<< endl;
	}
	fout.close();
}


CoarseGrainedPattern::~CoarseGrainedPattern()
{
}
