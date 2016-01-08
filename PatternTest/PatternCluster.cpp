#include "PatternCluster.h"



PatternCluster::PatternCluster():patterns(list<FineGrainedPattern*>()), semanticType(-1), objs(set<int>()), edges(set<Edge*>())
{
}

PatternCluster::PatternCluster(const PatternCluster & sourcePatternCluster, set<int>& trajObjs):patterns(list<FineGrainedPattern*>()),semanticType(sourcePatternCluster.semanticType),objs(set<int>()),edges(set<Edge*>())
{
	for each (FineGrainedPattern* fgpPtr in sourcePatternCluster.patterns)
	{
		set<int> tmp = set<int>();
		set_intersection(fgpPtr->clusterObjects.begin(), fgpPtr->clusterObjects.end(), trajObjs.begin(), trajObjs.end(), inserter(tmp, tmp.begin()));
		if (tmp.size() > 0) {
			patterns.push_back(fgpPtr);
			objs.insert(fgpPtr->clusterObjects.begin(), fgpPtr->clusterObjects.end());
			for each (auto edgeClusterPtr in fgpPtr->edgeClusterPattern)
			{
				edges.insert(edgeClusterPtr->clusterCoreEdge);
			}
		}
	}
}

void PatternCluster::insertPattern(FineGrainedPattern* pattern) {
	this->patterns.push_back(pattern);
	objs.insert(pattern->clusterObjects.begin(),pattern->clusterObjects.end());
	for each (auto edgeClusterPtr in pattern->edgeClusterPattern)
	{
		edges.insert(edgeClusterPtr->clusterCoreEdge);
	}
}

PatternCluster::~PatternCluster()
{
}
