#include"Cluster.h"

Cluster::Cluster(int time) :time(time){
	clusterObjects = set<GeoPoint*>();
}

bool Cluster::add(GeoPoint* point){
	if (clusterObjects.find(point) == clusterObjects.end()){
		clusterObjects.insert(point);
		return true;
	}
	else{
		return false;
	}	
}