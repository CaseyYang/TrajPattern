#pragma once
#include"Cluster.h"
#include "../../MapLibraries/Map.h"

class TimeSlice{
public:
	list<GeoPoint*> points;//ʱ��Ƭ���������Ĺ켣�����㼯��
	list<Cluster*> clusters;//ʱ��Ƭ���������ľ��༯��
	int time;//ʱ��Ƭ��Ӧ��ʱ��

	TimeSlice(int time);
	void clustering(Map& routeNetwork);

private:
	void dbscan(Map& routeNetwork);
	void expandCluster(Map& routeNetwork,GeoPoint* p, list<GeoPoint*> &neighbor, Cluster* cluster);
};