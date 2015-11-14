#pragma once
#include <stack>
#include <set>
#include <iterator>
#include <algorithm>
#include "TimeSlice.h"
#include "Map.h"

TimeSlice::TimeSlice(int time) :time(time) {
	clusters = list<Cluster*>();
	points = list<GeoPoint*>();
}

void TimeSlice::clustering(Map& routeNetwork) {
	dbscan(routeNetwork);
}

void TimeSlice::dbscan(Map& routeNetwork) {
	int index = 0;
	for (list<GeoPoint*>::iterator pointIter = points.begin(); pointIter != points.end(); pointIter++) {
		if (!(*pointIter)->visited) {
			(*pointIter)->visited = true;
			list<GeoPoint*> neighbors = list<GeoPoint*>();
			routeNetwork.getNearPointsInSameTimeStamp(*pointIter, DB_EPS, neighbors);
			if (neighbors.size() >= DB_MINPTS) {
				Cluster* cluster = new Cluster(time);
				expandCluster(routeNetwork, *pointIter, neighbors, cluster);
				clusters.push_back(cluster);
			}
		}
		index++;
	}
}

void TimeSlice::expandCluster(Map& routeNetwork, GeoPoint* p, list<GeoPoint*> &neighbor, Cluster* cluster) {
	if (cluster->add(p)) {
		p->clustered = true;;
		for (list<GeoPoint*>::iterator neighborIter = neighbor.begin(); neighborIter != neighbor.end(); neighborIter++) {
			set<GeoPoint*> tmpSet1 = set<GeoPoint*>(neighbor.begin(), neighbor.end());
			if (!(*neighborIter)->visited) {
				(*neighborIter)->visited = true;
				list<GeoPoint*> otherNeighbor = list<GeoPoint*>();
				routeNetwork.getNearPointsInSameTimeStamp(*neighborIter, DB_EPS, otherNeighbor);
				if (otherNeighbor.size() >= DB_MINPTS) {
					set<GeoPoint*> tmpSet2 = set<GeoPoint*>(otherNeighbor.begin(), otherNeighbor.end());
					set<GeoPoint*> tmpSet3 = set<GeoPoint*>();
					set_difference(tmpSet2.begin(), tmpSet2.end(), tmpSet1.begin(), tmpSet1.end(), inserter(tmpSet3, tmpSet3.begin()));
					neighbor.insert(neighbor.end(), tmpSet3.begin(), tmpSet3.end());
				}
			}
			if (!(*neighborIter)->clustered) {
				cluster->add((*neighborIter));
				(*neighborIter)->clustered = true;
			}
		}
	}
	else {
		cout << "出错：聚类中已存在" << endl;
		cout << "核心点：" << p->id << " " << p->objectId << endl;
		system("pause");
	}
}