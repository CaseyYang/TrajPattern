#include <set>
#include "GeoPoint.h"
using namespace std;

#define DB_EPS 10//DBSCAN�㷨��ָ��������뾶
#define DB_MINPTS 5//DBSCAN�㷨��ָ��������Ԫ�ظ���


class Cluster{
public:
	set<GeoPoint*> clusterObjects;
	int time;

	Cluster(int time);
	bool add(GeoPoint* point);
};