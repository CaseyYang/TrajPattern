#include "CoarseGrainedPattern.h"



CoarseGrainedPattern::CoarseGrainedPattern() :patternClusters(list<PatternCluster*>())
{
}

void CoarseGrainedPattern::outputCGP()
{
	int startType = patternClusters.front()->semanticType;
	int endType = patternClusters.back()->semanticType;
	int cluster1StartTimeStamp = patternClusters.front()->startTimeStamp;
	int cluster1EndTimeStamp = patternClusters.front()->endTimeStamp;
	int cluster2StartTimeStamp = patternClusters.back()->startTimeStamp;
	int cluster2EndTimeStamp = patternClusters.back()->endTimeStamp;
	stringstream ss;
	ss << "edges_" << startType << "_" << endType << "_"
		<< cluster1StartTimeStamp << "_" << cluster1EndTimeStamp << "_"
		<< cluster2StartTimeStamp << "_" << cluster2EndTimeStamp << ".txt";
	ofstream fout(ss.str());
	for each (PatternCluster* patternClusterPtr in patternClusters)
	{
		for each (auto edgePtr in patternClusterPtr->edges)
		{
			fout << edgePtr->id << endl;
		}
		fout << "-1" << endl;
	}
	fout.close();
}

void CoarseGrainedPattern::outputTimestamp()
{
	int startType = patternClusters.front()->semanticType;
	int endType = patternClusters.back()->semanticType;
	int cluster1StartTimeStamp = patternClusters.front()->startTimeStamp;
	int cluster1EndTimeStamp = patternClusters.front()->endTimeStamp;
	int cluster2StartTimeStamp = patternClusters.back()->startTimeStamp;
	int cluster2EndTimeStamp = patternClusters.back()->endTimeStamp;
	stringstream ss;
	ss << "time_" << startType << "_" << endType << "_" 
		<< cluster1StartTimeStamp << "_" << cluster1EndTimeStamp << "_" 
		<< cluster2StartTimeStamp << "_" << cluster2EndTimeStamp << ".txt";
	ofstream fout(ss.str());
	for each (PatternCluster* patternClusterPtr in patternClusters)
	{
		for each (auto patternPtr in patternClusterPtr->patterns)
		{
			fout << patternPtr->startTime << endl;
			fout << patternPtr->endTime << endl;
		}
		fout << "-1" << endl;
	}
	fout.close();
}

bool CoarseGrainedPattern::check()
{
	for each (auto patternClusterPtr in patternClusters)
	{
		set<FineGrainedPattern*> patterns = set<FineGrainedPattern*>();
		for each (auto patternPtr in patternClusterPtr->patterns)
		{
			if (patterns.find(patternPtr) == patterns.end()) {
				patterns.insert(patternPtr);
			}
			else {
				return false;
			}
		}
	}
	return true;
}


CoarseGrainedPattern::~CoarseGrainedPattern()
{
}
