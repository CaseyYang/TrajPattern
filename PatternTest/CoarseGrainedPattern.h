#pragma once
#include <list>
#include <string>
#include <fstream>
#include <sstream>
#include <time.h>
#include "PatternTimeSlot.h"

class CoarseGrainedPattern
{
public:
	list<PatternCluster*> patternClusters;
	CoarseGrainedPattern();
	void outputCGP();
	void outputTimestamp();
	bool check();
	~CoarseGrainedPattern();
};

