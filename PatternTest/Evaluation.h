#pragma once
#include "Map.h"
#include "EdgeCluster.h"

extern list<list<EdgeCluster*>> resultsList;

struct SubTraj{
	Edge* endEdge;//�ӹ켣��ǰ���һ��·��ָ��
	double dist;//�ӹ켣�������ȣ���λΪ�ף�
	int startTime;//�ӹ켣��ʼʱ�䣨��λΪ���ӣ�
	int duration;//�ӹ켣����ʱ�䣨��λΪ�룩

	//���캯��
	SubTraj(int startTime, Edge* edge);
	//�����ӹ켣�ĵ�ǰ���һ��·��ָ�룬�����³������Ⱥͳ���ʱ��
	void refresh(Edge* edge);
	//�����ӹ켣��ƽ���ٶȣ���λΪ��/��
	double calculateAverageSpeed();
	//����ӹ켣��Ϣ
	void outputSubTraj();
};

//������Ҫ���˵�·������
void filterInvalidEdgeSet();

//���������м��������а������ӹ켣��ƽ���ٶ�
void getAverageSpeed();

//ͳ�ƽ��·������
void getDistinctEdges();

//ͳ�ƽ��ʱ��ֲ�
void getTimeStatistic();

//���������м���
void outputResults(string fileName);