#pragma once
#include"Cluster.h"
#include "../../MapLibraries/Map.h"

class TimeSlice{
public:
	list<GeoPoint*> points;//时间片中所包含的轨迹采样点集合
	list<Cluster*> clusters;//时间片中所包含的聚类集合
	int time;//时间片对应的时间

	TimeSlice(int time);
	void clustering(Map& routeNetwork);

private:
	void dbscan(Map& routeNetwork);
	void expandCluster(Map& routeNetwork,GeoPoint* p, list<GeoPoint*> &neighbor, Cluster* cluster);
};