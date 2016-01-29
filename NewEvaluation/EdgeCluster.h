#pragma once
#include <set>
#include "../MapLibraries/Map.h"
using namespace std;

class EdgeCluster {
public:
	Edge* clusterCoreEdge;//核心路段指针
	set<int> clusterObjects;//路段聚类所包含的轨迹集合
	map<Edge*, int> nextEdgeCounts;//下一路段聚类的计数
	list<Edge*> priorCanadidates;//最有可能用来扩展的路段聚类
	int time;//聚类所属时间
	int k;//表示已经扩展的轨迹数
	bool assigned;//表示路段聚类是否已经用于扩展

	EdgeCluster(Edge* edge, int time);//构造函数，参数为路段指针和聚类所属时间
	bool add(int objectId);//在路段聚类中添加一条轨迹，返回true表示添加成功；返回false表示该条轨迹已存在
	void refreshNextEdgeCounts(Edge* nextEdge);//用下一条匹配路段来更新nextEdgeCounts
	void ascertainPriorCanadidates();//从nextEdgeCounts集合中找出最有可能用来扩展的路段聚类，标准是扩展路段聚类计数不小于本路段聚类元素数乘以最小相似度minSimiliarity
};