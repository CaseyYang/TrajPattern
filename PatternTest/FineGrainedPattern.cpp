#include "FineGrainedPattern.h"

FineGrainedPattern::FineGrainedPattern(list<EdgeCluster*> edgeClusters):startTime(edgeClusters.front()->time),endTime(edgeClusters.back()->time)
{
	edgeClusterPattern = edgeClusters;
	for each (EdgeCluster* edgeClusterPtr in edgeClusters)
	{
		this->clusterObjects.insert(edgeClusterPtr->clusterObjects.begin(), edgeClusterPtr->clusterObjects.end());
	}
}
