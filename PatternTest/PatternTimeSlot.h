#pragma once
#include <vector>
#include "FineGrainedPattern.h"
#include "PatternCluster.h"
using namespace std;

//时间聚类算法中的一个时间簇
class PatternTimeSlot
{
public:
	vector<FineGrainedPattern*> patterns;	//时间簇中所有元素对应的细粒度轨迹模式
	vector<int> timeStamps;					//每个轨迹模式的时间中值作为时间簇中的一个元素
	int center;								//时间簇中心
	double SSE;
	list<PatternCluster*> patternClusters;	//时间簇所包含的轨迹模式聚类
	PatternTimeSlot();
	PatternTimeSlot(list<FineGrainedPattern*>&patterns);
	void insertPattern(FineGrainedPattern* pattern);	//向时间簇中增加一个元素
	double calcSSE();									//计算时间簇的SSE（误差平方和）
	void clear();										//清空时间簇中所有数据
	void outputTimeStamps(string filePath);				//输出所有的中值时间戳，调试用
	~PatternTimeSlot();
};

