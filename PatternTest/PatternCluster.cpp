#include "PatternCluster.h"



PatternCluster::PatternCluster():patterns(list<FineGrainedPattern*>()), semanticType(-1), objs(set<int>())
{
}

void PatternCluster::insertPattern(FineGrainedPattern* pattern) {
	this->patterns.push_back(pattern);
	objs.insert(pattern->clusterObjects.begin(),pattern->clusterObjects.end());
}

PatternCluster::~PatternCluster()
{
}
