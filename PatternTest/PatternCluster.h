#pragma once
#include "FineGrainedPattern.h"

//轨迹模式聚类
class PatternCluster
{
public:
	list<FineGrainedPattern*> patterns;	//组成轨迹模式聚类的各条轨迹模式
	int semanticType;					//轨迹模式聚类的语义类型
	int startTimeStamp;					//轨迹模式聚类起始时间戳（实验统计用）
	int endTimeStamp;					//轨迹模式聚类终止时间戳（实验统计用）
	PatternCluster();
	void insertPattern(FineGrainedPattern* pattern);
	~PatternCluster();
};

