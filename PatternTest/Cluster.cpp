#include"Cluster.h"

Cluster::Cluster(int time) :time(time){
	clusterObjects = list<TrajSamplePoint*>();
}

