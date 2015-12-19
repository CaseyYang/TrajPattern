#pragma once
#include <vector>
#include "FineGrainedPattern.h"
using namespace std;

//时间聚类算法中的一个时间簇
class PatternTimeSlot
{
public:
	vector<FineGrainedPattern*> patterns;	//时间簇中所有元素对应的细粒度轨迹模式
	vector<int> timeStamps;					//每个轨迹模式的时间中值作为时间簇中的一个元素
	int center;								//时间簇中心
	double SSE;
	PatternTimeSlot();
	PatternTimeSlot(list<FineGrainedPattern*>&patterns);
	void InsertPattern(FineGrainedPattern* pattern);
	~PatternTimeSlot();
};

