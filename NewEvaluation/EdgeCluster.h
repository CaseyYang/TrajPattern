#pragma once
#include <set>
#include "../MapLibraries/Map.h"
using namespace std;

class EdgeCluster {
public:
	Edge* clusterCoreEdge;//����·��ָ��
	set<int> clusterObjects;//·�ξ����������Ĺ켣����
	map<Edge*, int> nextEdgeCounts;//��һ·�ξ���ļ���
	list<Edge*> priorCanadidates;//���п���������չ��·�ξ���
	int time;//��������ʱ��
	int k;//��ʾ�Ѿ���չ�Ĺ켣��
	bool assigned;//��ʾ·�ξ����Ƿ��Ѿ�������չ

	EdgeCluster(Edge* edge, int time);//���캯��������Ϊ·��ָ��;�������ʱ��
	bool add(int objectId);//��·�ξ��������һ���켣������true��ʾ��ӳɹ�������false��ʾ�����켣�Ѵ���
	void refreshNextEdgeCounts(Edge* nextEdge);//����һ��ƥ��·��������nextEdgeCounts
	void ascertainPriorCanadidates();//��nextEdgeCounts�������ҳ����п���������չ��·�ξ��࣬��׼����չ·�ξ��������С�ڱ�·�ξ���Ԫ����������С���ƶ�minSimiliarity
};