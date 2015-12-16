#pragma once

#include<list>
#include"EdgeCluster.h"
using namespace std;

//细粒度pattern（NDBC工作成果）
class FineGrainedPattern {
public:
	int startTime;//细粒度pattern起始时间
	int endTime;//细粒度pattern终止时间
	set<int> clusterObjects;//细粒度pattern中包含的轨迹
	list<EdgeCluster*> edgeClusterPattern;//组成细粒度pattern的路段聚类序列
	FineGrainedPattern(list<EdgeCluster*> edgeClusters);//构造函数，把原来的EdgeCluster数组变为一个FineGraninedPattern实例
};