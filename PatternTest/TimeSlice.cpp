#pragma once
#include <stack>
#include <set>
#include <iterator>
#include <algorithm>
#include "TimeSlice.h"
#include "Map.h"

TimeSlice::TimeSlice(int time) :time(time){
	clusters = list<Cluster*>();
	points = list<GeoPoint*>();
}

void TimeSlice::clustering(Map& routeNetwork){
	dbscan(routeNetwork);
	//DBSCAN算法
	//第一步：将所有点标记为核心点、边界点或噪声点
	//for (list<GeoPoint*>::iterator pointIter = points.begin(); pointIter != points.end(); pointIter++){
	//	list<GeoPoint*> neighbors = list<GeoPoint*>();
	//	routeNetwork.getNearPoints((*pointIter)->lat, (*pointIter)->lon, DB_EPS, neighbors);
	//	if (neighbors.size() >= DB_MINPTS){
	//		(*pointIter)->properties = "core";//邻域内邻居点个数超过DB_MINPTS，是核心点
	//		for (list<GeoPoint*>::iterator neighborIter = neighbors.begin(); neighborIter != neighbors.end(); neighborIter++){
	//			if ((*neighborIter)->properties != "core"){
	//				(*neighborIter)->properties = "neighbor";//核心点的邻居都是邻居点
	//			}
	//		}
	//	}
	//	else{
	//		if ((*pointIter)->properties != "neighbor"){
	//			(*pointIter)->properties = "noise";//邻域内邻居点个数不到DB_MINPTS，是噪声点，直接删除
	//		}
	//	}
	//}

	//for (list<GeoPoint*>::iterator pointIter = points.begin(); pointIter != points.end(); pointIter++)
	//{
	//	if ((*pointIter)->properties == ""){
	//		list<GeoPoint*> neighbors = list<GeoPoint*>();
	//		routeNetwork.getNearPoints((*pointIter)->lat, (*pointIter)->lon, DB_EPS, neighbors);
	//		neighbors.remove((*pointIter));
	//		if (neighbors.size() >= DB_MINPTS){
	//			(*pointIter)->properties = "core";
	//			Cluster* cluster = new Cluster(time);
	//			cluster->add((*pointIter));
	//			stack<GeoPoint*> pointStack = stack<GeoPoint*>();
	//			pointStack.push((*pointIter));
	//			while (pointStack.size() > 0){
	//				GeoPoint* curPoint = pointStack.top();
	//				pointStack.pop();
	//				if (curPoint->properties == ""){
	//					list<GeoPoint*> curNeighbors = list<GeoPoint*>();
	//					routeNetwork.getNearPoints(curPoint->lat, curPoint->lon, DB_EPS, curNeighbors);
	//					curNeighbors.remove(curPoint);
	//					if (curNeighbors.size() >= DB_MINPTS){
	//						curPoint->properties = "core";
	//						for (list<GeoPoint*>::iterator curNeighborIter = curNeighbors.begin(); curNeighborIter != curNeighbors.end(); curNeighborIter++){

	//						}
	//					}
	//				}
	//			}
	//			for (list<GeoPoint*>::iterator neighborIter = neighbors.begin(); neighborIter != neighbors.end(); neighborIter++)
	//			{
	//				if ((*neighborIter)->properties == ""){
	//					pointStack.push(neighborPoint);
	//					continue;
	//				}
	//				if (neighborPoint->properties == "noice"){
	//					neighborPoint->properties = "neighbor";
	//					cluster->clusterObjects.push_back(neighborPoint);
	//					continue;
	//				}
	//				if (neighborPoint->properties == "core"&&neighborPoint != point){
	//					cout << "DBSCAN问题1：新的核心点周边出现旧的核心点" << endl;
	//					cout << "旧的核心点：" << "id=" << neighborPoint->id << ", objectId=" << neighborPoint->objectId << endl;
	//					cout << "新的核心点" << "id=" << point->id << "，objectId=" << point->objectId << endl;
	//					system("pause");
	//				}
	//			}
	//			while (pointStack.size() > 0){
	//				GeoPoint* curPoint = pointStack.top();
	//				pointStack.pop();
	//				if (curPoint->properties == ""){
	//					cluster->clusterObjects.push_back(curPoint);
	//					list<GeoPoint*> otherNeighbors = list<GeoPoint*>();
	//					routeNetwork.getNearPoints(curPoint->lat, curPoint->lon, DB_EPS, otherNeighbors);
	//					if (otherNeighbors.size() >= DB_MINPTS){
	//						curPoint->properties = "core";
	//						for (list<GeoPoint*>::iterator neighborIter = otherNeighbors.begin(); neighborIter != otherNeighbors.end(); neighborIter++)
	//						{
	//							GeoPoint* otherNeighborPoint = *neighborIter;
	//							if (otherNeighborPoint->properties == ""){
	//								pointStack.push(otherNeighborPoint);
	//								continue;
	//							}
	//							if (otherNeighborPoint->properties == "noice"){
	//								otherNeighborPoint->properties = "neighbor";
	//								cluster->clusterObjects.push_back(otherNeighborPoint);
	//								continue;
	//							}
	//						}
	//					}
	//					else{
	//						curPoint->properties = "neighbor";
	//					}
	//					continue;
	//				}
	//				if (curPoint->properties == "noice"){
	//					cout << "DBSCAN问题2：不该出现噪声点" << endl;
	//					system("pause");
	//				}
	//			}
	//			clusters.push_back(cluster);
	//		}
	//	}
	//}
}

void TimeSlice::dbscan(Map& routeNetwork){
	int index = 0;
	for (list<GeoPoint*>::iterator pointIter = points.begin(); pointIter != points.end(); pointIter++){
		if (!(*pointIter)->visited){
			(*pointIter)->visited = true;
			list<GeoPoint*> neighbors = list<GeoPoint*>();
			routeNetwork.getNearPointsInSameTimeStamp(*pointIter, DB_EPS, neighbors);
			if (neighbors.size() >= DB_MINPTS){
				Cluster* cluster = new Cluster(time);
				expandCluster(routeNetwork, *pointIter, neighbors, cluster);
				clusters.push_back(cluster);
			}
		}
		index++;
	}
}

void TimeSlice::expandCluster(Map& routeNetwork, GeoPoint* p, list<GeoPoint*> &neighbor, Cluster* cluster){
	if (cluster->add(p)){
		p->clustered = true;;
		for (list<GeoPoint*>::iterator neighborIter = neighbor.begin(); neighborIter != neighbor.end(); neighborIter++){
			set<GeoPoint*> tmpSet1 = set<GeoPoint*>(neighbor.begin(), neighbor.end());
			if (!(*neighborIter)->visited){
				(*neighborIter)->visited = true;
				list<GeoPoint*> otherNeighbor = list<GeoPoint*>();
				routeNetwork.getNearPointsInSameTimeStamp(*neighborIter, DB_EPS, otherNeighbor);
				if (otherNeighbor.size() >= DB_MINPTS){
					set<GeoPoint*> tmpSet2 = set<GeoPoint*>(otherNeighbor.begin(), otherNeighbor.end());
					set<GeoPoint*> tmpSet3 = set<GeoPoint*>();
					set_difference(tmpSet2.begin(), tmpSet2.end(), tmpSet1.begin(), tmpSet1.end(), inserter(tmpSet3, tmpSet3.begin()));
					neighbor.insert(neighbor.end(), tmpSet3.begin(), tmpSet3.end());
				}
			}
			if (!(*neighborIter)->clustered){
				cluster->add((*neighborIter));
				(*neighborIter)->clustered = true;
			}
		}
	}
	else{
		cout << "出错：聚类中已存在" << endl;
		cout << "核心点：" << p->id << " " << p->objectId << endl;
		system("pause");
	}
}