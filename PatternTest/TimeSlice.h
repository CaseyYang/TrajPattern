#pragma once
#include"Cluster.h"
#include"Map.h"

class TimeSlice{
public:
	list<GeoPoint*> points;//ʱ��Ƭ���������Ĺ켣�����㼯��
	list<Cluster*> clusters;//ʱ��Ƭ���������ľ��༯��
	int time;//ʱ��Ƭ��Ӧ��ʱ��

	TimeSlice(int time);
	void clustering(Map& routeNetwork);
};