#pragma once
#include <iterator>
#include "FineGrainedPattern.h"

//轨迹模式聚类
class PatternCluster
{
public:
	set<FineGrainedPattern*> patterns;	//组成轨迹模式聚类的各条轨迹模式
	int semanticType;					//轨迹模式聚类的语义类型
	set<int> objs;						//轨迹模式聚类包含的轨迹
	set<Edge*> edges;					//轨迹模式聚类包含的路段
	int startTimeStamp;					//轨迹模式聚类起始时间戳（实验统计用）
	int endTimeStamp;					//轨迹模式聚类终止时间戳（实验统计用）
	PatternCluster();
	PatternCluster(const PatternCluster& sourcePatternCluster, set<int>& trajObjs);//复制构造函数，只包含trajObjs中有的轨迹组成的Pattern
	void insertPattern(FineGrainedPattern* pattern);
	~PatternCluster();
};

