#include"Cluster.h"

Cluster::Cluster(int time) :time(time) {
	clusterObjects = set<GeoPoint*>();
	objectIds = set<int>();
}

bool Cluster::add(GeoPoint* point) {
	if (clusterObjects.find(point) == clusterObjects.end() && objectIds.find(point->id) == objectIds.end()) {
		clusterObjects.insert(point);
		objectIds.insert(point->id);
		return true;
	}
	else {
		return false;
	}
}