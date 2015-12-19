#include "PatternCluster.h"



PatternCluster::PatternCluster():patterns(list<FineGrainedPattern*>()), semanticType(-1)
{
}

void PatternCluster::insertPattern(FineGrainedPattern* pattern) {
	this->patterns.push_back(pattern);
}

PatternCluster::~PatternCluster()
{
}
