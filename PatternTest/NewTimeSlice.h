#pragma once
#include <set>
#include <map>
#include "EdgeCluster.h"
#include "Map.h"

class NewTimeSlice{
public:
	set<int> sliceObjects;//时间片中所包含的轨迹集合
	map<Edge*,EdgeCluster*> clusters;//时间片中所包含的聚类集合
	int time;//时间片对应的时间

	NewTimeSlice(int time);//构造函数，参数为时间片所表示的时间
	bool add(int objectId,Edge* edge);//向时间片中给定路段添加一条轨迹；若路段聚类已存在则直接添加，否则先创建路段聚类，再添加元素
};