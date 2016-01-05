#include "PatternCluster.h"



PatternCluster::PatternCluster():patterns(list<FineGrainedPattern*>()), semanticType(-1), objs(set<int>())
{
}

PatternCluster::PatternCluster(const PatternCluster & sourcePatternCluster, set<int>& trajObjs):patterns(list<FineGrainedPattern*>()),semanticType(sourcePatternCluster.semanticType),objs(set<int>())
{
	for each (FineGrainedPattern* fgpPtr in sourcePatternCluster.patterns)
	{
		set<int> tmp = set<int>();
		set_intersection(fgpPtr->clusterObjects.begin(), fgpPtr->clusterObjects.end(), trajObjs.begin(), trajObjs.end(), inserter(tmp, tmp.begin()));
		if (tmp.size() > 0) {
			patterns.push_back(fgpPtr);
			objs.insert(fgpPtr->clusterObjects.begin(), fgpPtr->clusterObjects.end());
		}
	}
}

void PatternCluster::insertPattern(FineGrainedPattern* pattern) {
	this->patterns.push_back(pattern);
	objs.insert(pattern->clusterObjects.begin(),pattern->clusterObjects.end());
}

PatternCluster::~PatternCluster()
{
}
