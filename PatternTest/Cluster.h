#pragma once
#include <list>
#include "GeoPoint.h"
using namespace std;

#define DB_EPS 10//DBSCAN�㷨��ָ��������뾶
#define DB_MINPTS 5//DBSCAN�㷨��ָ��������Ԫ�ظ���

class Cluster{
public:
	list<GeoPoint*> clusterObjects;
	int time;

	Cluster(int time);
};