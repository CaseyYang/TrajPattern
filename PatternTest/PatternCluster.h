#pragma once
#include "FineGrainedPattern.h"

class PatternCluster
{
public:
	list<FineGrainedPattern*> patterns;
	int semanticType;
	PatternCluster();
	void insertPattern(FineGrainedPattern* pattern);
	~PatternCluster();
};

