#include <set>
#include "GeoPoint.h"
using namespace std;

#define DB_EPS 10//DBSCAN算法中指定的邻域半径
#define DB_MINPTS 5//DBSCAN算法中指定的最少元素个数


class Cluster{
public:
	set<GeoPoint*> clusterObjects;//聚类所包含的轨迹采样点集合
	int time;//聚类所属时间

	Cluster(int time);//构造函数，参数为聚类所属时间
	bool add(GeoPoint* point);//在聚类中添加一个轨迹采样点，返回true表示添加成功；返回false表示该采样点已存在
};