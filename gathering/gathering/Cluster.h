#include <set>
#include "../../MapLibraries/GeoPoint.h"
using namespace std;

#define DB_EPS 200//DBSCAN�㷨��ָ��������뾶
#define DB_MINPTS 5//DBSCAN�㷨��ָ��������Ԫ�ظ���


class Cluster{
public:
	set<GeoPoint*> clusterObjects;
	int time;
	bool flag;

	Cluster(int time);
	bool add(GeoPoint* point);
};