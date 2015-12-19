#include "FineGrainedPattern.h"

FineGrainedPattern::FineGrainedPattern(list<EdgeCluster*> edgeClusters):startTime(edgeClusters.front()->time),endTime(edgeClusters.back()->time)
{
	edgeClusterPattern = edgeClusters;
	for each (EdgeCluster* edgeClusterPtr in edgeClusters)
	{
		for each (int objectId in edgeClusterPtr->clusterObjects)
		{
			this->clusterObjects.insert(objectId);
		}
	}
}
