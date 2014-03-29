#include <stack>
#include "TimeSlice.h"
#include "Map.h"

TimeSlice::TimeSlice(int time) :time(time){
	clusters = list<Cluster*>();
	points = list<GeoPoint*>();
}

void TimeSlice::clustering(Map& routeNetwork){
	for each (GeoPoint* point in points)
	{
		if (point->properties == ""){
			list<GeoPoint*> neighbors = list<GeoPoint*>();
			routeNetwork.getNearPoints(point->lat, point->lon, DB_EPS, neighbors);
			if (neighbors.size() >= DB_MINPTS){
				point->properties = "core";
				Cluster* cluster = new Cluster(time);
				cluster->clusterObjects.push_back(point);
				stack<GeoPoint*> pointStack = stack<GeoPoint*>();
				for each (GeoPoint* neighborPoint in neighbors)
				{
					if (point->properties == ""){
						pointStack.push(neighborPoint);
						continue;
					}
					if (point->properties == "noice"){
						point->properties = "neighbor";
						cluster->clusterObjects.push_back(point);
						continue;
					}
					if (point->properties == "core"){
						cout << "DBSCAN问题1：新的核心点周边出现旧的核心点" << endl;
						system("pause");
					}
				}
				while (pointStack.size() > 0){
					GeoPoint* curPoint = pointStack.top();
					pointStack.pop();
					if (curPoint->properties == ""){
						cluster->clusterObjects.push_back(curPoint);
						list<GeoPoint*> otherNeighbors = list<GeoPoint*>();
						routeNetwork.getNearPoints(curPoint->lat, curPoint->lon, DB_EPS, otherNeighbors);
						if (otherNeighbors.size() >= DB_MINPTS){
							curPoint->properties = "core";
							for each (GeoPoint*  otherNeighborPoint in otherNeighbors)
							{
								if (otherNeighborPoint->properties == ""){
									pointStack.push(otherNeighborPoint);
									continue;
								}
								if (otherNeighborPoint->properties == "noice"){
									otherNeighborPoint->properties = "neighbor";
									cluster->clusterObjects.push_back(otherNeighborPoint);
									continue;
								}
							}
						}
						else{
							curPoint->properties = "neighbor";
						}
						continue;
					}
					if (curPoint->properties == "noice"){
						cout << "DBSCAN问题2：不该出现噪声点" << endl;
						system("pause");
					}
				}
				clusters.push_back(cluster);
			}
			else{
				point->properties = "noice";
			}
		}
	}
}