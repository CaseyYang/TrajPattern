#include "EdgeCluster.h"
#include "Parameters.h"
using namespace std;

//���캯��������Ϊ·��ָ��;�������ʱ��
EdgeCluster::EdgeCluster(Edge* edge, int time) :clusterCoreEdge(edge), time(time) {
	clusterObjects = set<int>();
	nextEdgeCounts = map<Edge*, int>();
	priorCanadidates = list<Edge*>();
	k = 0;
	assigned = false;
}

//��·�ξ��������һ���켣������true��ʾ��ӳɹ�������false��ʾ�����켣�Ѵ���
bool EdgeCluster::add(int objectId) {
	if (clusterObjects.find(objectId) == clusterObjects.end()) {
		clusterObjects.insert(objectId);
		return true;
	}
	else {
		return false;
	}
}

//����һ��ƥ��·��������nextEdgeCounts
void EdgeCluster::refreshNextEdgeCounts(Edge* nextEdge) {
	if (nextEdgeCounts.find(nextEdge) == nextEdgeCounts.end()) {
		nextEdgeCounts.insert(make_pair(nextEdge, 0));
	}
	nextEdgeCounts.at(nextEdge)++;
}

//��nextEdgeCounts�������ҳ����п���������չ��·�ξ��࣬��׼����չ·�ξ��������С�ڱ�·�ξ���Ԫ����������С���ƶ�minSimiliarity
void EdgeCluster::ascertainPriorCanadidates() {
	for each (pair<Edge*, int> pair in nextEdgeCounts)
	{
		if (pair.second >= clusterObjects.size()*DE_MINSIMILARITY) {
			priorCanadidates.push_back(pair.first);
		}
	}
}