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
};

int invalidEdges[] = { 55402, 27454, 27489, 55435 };
set<int> invalidEdgeSet = set<int>();
set<Edge*> distinctEdges = set<Edge*>();//���н��·�μ���

//������Ҫ���˵�·������
void filterInvalidEdgeSet();

//���������м��������а������ӹ켣��ƽ���ٶ�
vector<double> getAverageSpeed();

//ͳ�ƽ��·������
void getDistinctEdges();

//ͳ�ƽ��ʱ��ֲ�
void getTimeStatistic();