#pragma once
#include <set>
#include <map>
#include "EdgeCluster.h"
#include "Map.h"

class NewTimeSlice{
public:
	set<int> sliceObjects;//ʱ��Ƭ���������Ĺ켣����
	map<Edge*,EdgeCluster*> clusters;//ʱ��Ƭ���������ľ��༯��
	int time;//ʱ��Ƭ��Ӧ��ʱ��

	NewTimeSlice(int time);//���캯��������Ϊʱ��Ƭ����ʾ��ʱ��
	bool add(int objectId,Edge* edge);//��ʱ��Ƭ�и���·�����һ���켣����·�ξ����Ѵ�����ֱ����ӣ������ȴ���·�ξ��࣬�����Ԫ��
};