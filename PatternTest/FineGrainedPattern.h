#pragma once

#include<list>
#include"EdgeCluster.h"
using namespace std;

//ϸ����pattern��NDBC�����ɹ���
class FineGrainedPattern {
public:
	int startTime;//ϸ����pattern��ʼʱ��
	int endTime;//ϸ����pattern��ֹʱ��
	set<int> clusterObjects;//ϸ����pattern�а����Ĺ켣
	list<EdgeCluster*> edgeClusterPattern;//���ϸ����pattern��·�ξ�������
	FineGrainedPattern(list<EdgeCluster*> edgeClusters);//���캯������ԭ����EdgeCluster�����Ϊһ��FineGraninedPatternʵ��
};