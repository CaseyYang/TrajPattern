#pragma once
#include <list>
#include "PatternTimeSlot.h"

class CoarseGrainedPattern
{
public:
	list<PatternCluster*> patternClusters;
	CoarseGrainedPattern();
	~CoarseGrainedPattern();
};

