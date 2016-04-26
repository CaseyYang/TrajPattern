#pragma once
#include <set>
#include "../MapLibraries/GeoPoint.h"
using namespace std;

#define DB_EPS 10//DBSCAN�㷨��ָ��������뾶
#define DB_MINPTS 5//DBSCAN�㷨��ָ��������Ԫ�ظ���


class Cluster {
public:
	set<GeoPoint*> clusterObjects;//�����������Ĺ켣�����㼯��
	set<int> objectIds;//��������Ĺ켣���������ڵĹ켣Id����
	int time;//��������ʱ��

	Cluster(int time);//���캯��������Ϊ��������ʱ��
	bool add(GeoPoint* point);//�ھ���������һ���켣�����㣬����true��ʾ���ӳɹ�������false��ʾ�ò������Ѵ���
};