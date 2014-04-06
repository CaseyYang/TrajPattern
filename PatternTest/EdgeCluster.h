#include <set>
#include "Map.h"
using namespace std;

class EdgeCluster{
public:
	Edge* clusterCoreEdge;//核心路段指针
	set<int> clusterObjects;//路段聚类所包含的轨迹集合
	int time;//聚类所属时间

	EdgeCluster(Edge* edge,int time);//构造函数，参数为路段指针和聚类所属时间
	bool add(int objectId);//在路段聚类中添加一条轨迹，返回true表示添加成功；返回false表示该条轨迹已存在
};