#include <set>
#include "Map.h"
using namespace std;

class EdgeCluster{
public:
	Edge* clusterCoreEdge;//����·��ָ��
	set<int> clusterObjects;//·�ξ����������Ĺ켣����
	int time;//��������ʱ��

	EdgeCluster(Edge* edge,int time);//���캯��������Ϊ·��ָ��;�������ʱ��
	bool add(int objectId);//��·�ξ��������һ���켣������true��ʾ��ӳɹ�������false��ʾ�����켣�Ѵ���
};