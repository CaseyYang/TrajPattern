#include <set>
#include "GeoPoint.h"
using namespace std;

#define DB_EPS 10//DBSCAN算法中指定的邻域半径
#define DB_MINPTS 5//DBSCAN算法中指定的最少元素个数


class Cluster{
public:
	set<GeoPoint*> clusterObjects;
	int time;

	Cluster(int time);
	bool add(GeoPoint* point);
};