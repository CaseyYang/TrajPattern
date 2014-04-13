#include "Map.h"
#include "EdgeCluster.h"

extern list<list<EdgeCluster*>> resultsList;

struct SubTraj{
	Edge* endEdge;//子轨迹当前最后一个路段指针
	double dist;//子轨迹持续长度（单位为米）
	int startTime;//子轨迹开始时间（单位为分钟）
	int duration;//子轨迹持续时间（单位为秒）

	//构造函数
	SubTraj(int startTime, Edge* edge);
	//更新子轨迹的当前最后一个路段指针，并更新持续长度和持续时间
	void refresh(Edge* edge);
	//返回子轨迹的平均速度，单位为米/秒
	double calculateAverageSpeed();
};

int invalidEdges[] = { 55402, 27454, 27489, 55435 };
set<int> invalidEdgeSet = set<int>();
set<Edge*> distinctEdges = set<Edge*>();//序列结果路段集合

//构造需要过滤的路段序列
void filterInvalidEdgeSet();

//计算结果序列集合中所有包含的子轨迹的平均速度
vector<double> getAverageSpeed();

//统计结果路段数量
void getDistinctEdges();

//统计结果时间分布
void getTimeStatistic();