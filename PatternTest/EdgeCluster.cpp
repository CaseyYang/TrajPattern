#include "EdgeCluster.h"
#include "Parameters.h"
using namespace std;

//构造函数，参数为路段指针和聚类所属时间
EdgeCluster::EdgeCluster(Edge* edge, int time) :clusterCoreEdge(edge), time(time) {
	clusterObjects = set<int>();
	nextEdgeCounts = map<Edge*, int>();
	priorCanadidates = list<Edge*>();
	k = 0;
	assigned = false;
}

//在路段聚类中添加一条轨迹，返回true表示添加成功；返回false表示该条轨迹已存在
bool EdgeCluster::add(int objectId) {
	if (clusterObjects.find(objectId) == clusterObjects.end()) {
		clusterObjects.insert(objectId);
		return true;
	}
	else {
		return false;
	}
}

//用下一条匹配路段来更新nextEdgeCounts
void EdgeCluster::refreshNextEdgeCounts(Edge* nextEdge) {
	if (nextEdgeCounts.find(nextEdge) == nextEdgeCounts.end()) {
		nextEdgeCounts.insert(make_pair(nextEdge, 0));
	}
	nextEdgeCounts.at(nextEdge)++;
}

//从nextEdgeCounts集合中找出最有可能用来扩展的路段聚类，标准是扩展路段聚类计数不小于本路段聚类元素数乘以最小相似度minSimiliarity
void EdgeCluster::ascertainPriorCanadidates() {
	for each (pair<Edge*, int> pair in nextEdgeCounts)
	{
		if (pair.second >= clusterObjects.size()*DE_MINSIMILARITY) {
			priorCanadidates.push_back(pair.first);
		}
	}
}