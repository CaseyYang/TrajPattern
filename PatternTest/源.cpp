#include <iostream>
#include "ReadInTrajs.h"
#include "Map.h"
#include "TimeSlice.h"
#include "NewTimeSlice.h"
#include "Parameters.h"
using namespace std;

Map routeNetwork;
string filePath = "E:\\Document\\Subjects\\Computer\Data\\�¼��¹켣����\\";
vector<NewTimeSlice*> timeSlices;
list<list<EdgeCluster*>> resultsList;//���

//�Ա�ʵ��׼����������ȡ�켣�ļ�����������������
void clusterDemo(){
	vector<TimeSlice*> timeSlices = vector<TimeSlice*>(1440);//��ʼ��ʱ��Ƭ����
	int timeStamp = 0;
	for (int timeStamp = 0; timeStamp < 1440; timeStamp++){
		timeSlices.at(timeStamp) = new TimeSlice(timeStamp);
	}
	scanTrajFolder(filePath, timeSlices);//����켣
	cout << "�������й켣" << endl;
	int outIndexCount = 0;
	for each (TimeSlice* timeSlice in timeSlices)//�Թ켣�����㽨������
	{
		for each (GeoPoint* point in (*timeSlice).points)
		{
			if (!routeNetwork.insertPoint(point)){
				outIndexCount++;
			}
		}
	}
	cout << "�Թ켣����������ϣ�" << endl;
	cout << "����" << outIndexCount << "����������������Χ��" << endl;
	for each (TimeSlice* timeSlice in timeSlices)
	{
		timeSlice->clustering(routeNetwork);
	}
}

//ʵ��׼����������ȡ��ͼƥ��������ɸ�ʱ��Ƭ��·�ξ���
void edgeCluster(){
	timeSlices = vector<NewTimeSlice*>(1440);//��ʼ��ʱ��Ƭ����
	int timeStamp = 0;
	for (int timeStamp = 0; timeStamp < 1440; timeStamp++){
		timeSlices.at(timeStamp) = new NewTimeSlice(timeStamp);
	}
	scanMapMatchingResultFolder(filePath, timeSlices, routeNetwork);//�����ͼƥ�����ļ������ʱ��Ƭ��·�ξ���
	cout << "�������е�ͼƥ����" << endl;
	for (auto timeSlice : timeSlices)
	{
		for (auto edgeCluster : timeSlice->clusters){
			edgeCluster.second->ascertainPriorCanadidates();
		}
	}
}

//�����������жϸ���������·�ξ����Ƿ�������չ���������㷵��true�����򷵻�false
bool couldExtendOrNot(set<int> &set1, set<int> &set2){
	set<int> unionResult = set<int>();
	set_union(set1.begin(), set1.end(), set2.begin(), set2.end(), unionResult.begin());
	set<int> intersectionResult = set<int>();
	set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(), intersectionResult.begin());
	double similarity = (intersectionResult.size() + 0.0) / unionResult.size();
	return similarity >= DE_MINSIMILARITY;
}

//��������������һʱ��Ƭ����չ������·�ξ���
list<EdgeCluster*> extendDensityEdges(EdgeCluster* edgeCluster){
	list<EdgeCluster*> result = list<EdgeCluster*>();
	if (edgeCluster->time < timeSlices.size() - 1){
		for (auto tmpCluster : timeSlices.at(edgeCluster->time + 1)->clusters)
		{
			//������СԪ�ظ��������Ϳ���չ������
			if (tmpCluster.second->clusterObjects.size() >= DE_MINOBJECTS&&couldExtendOrNot(edgeCluster->clusterObjects, tmpCluster.second->clusterObjects)){
				tmpCluster.second->assigned = true;
				result.push_back(tmpCluster.second);
			}
		}
	}
	return result;
}

//naive����
list<list<EdgeCluster*>> naiveMethod(){
	resultsList = list<list<EdgeCluster*>>();
	list<list<EdgeCluster*>> canadidates = list<list<EdgeCluster*>>();//��ѡ���м���
	for (auto timeSlice : timeSlices){
		list<list<EdgeCluster*>> newCanadidates = list<list<EdgeCluster*>>();//�µĺ�ѡ���м���
		for (auto canadidate : canadidates){
			EdgeCluster* lastSnapshotCluster = canadidate.back();
			list<EdgeCluster*> assignedEdgeClusters = extendDensityEdges(lastSnapshotCluster);
			if (assignedEdgeClusters.size() == 0){
				if (canadidate.size() >= DE_DURATIVE){//�������������
					resultsList.push_back(canadidate);
				}
			}
			else{
				for (auto assignedEdgeCluster : assignedEdgeClusters){
					newCanadidates.push_back(canadidate);
					newCanadidates.back().push_back(assignedEdgeCluster);
				}
			}
		}
		for (auto edgeCluster : timeSlice->clusters){
			//��ǰʱ��Ƭ�е�·�ξ���δ������չ��������СԪ�ظ�������������Ϊ�µ�һ����ѡ����
			if ((!edgeCluster.second->assigned) && edgeCluster.second->clusterObjects.size()>=DE_MINOBJECTS){
				edgeCluster.second->assigned = true;
				list<EdgeCluster*> densityEdges = list<EdgeCluster*>();
				densityEdges.push_back(edgeCluster.second);
				newCanadidates.push_back(densityEdges);
			}
		}
		canadidates = newCanadidates;//���º�ѡ���м���
	}
	return resultsList;
}

//����kֵ��֦�ķ���
list<list<EdgeCluster*>> methodWithKPruning(){
	resultsList = list<list<EdgeCluster*>>();
	list<list<EdgeCluster*>> canadidates = list<list<EdgeCluster*>>();//��ѡ���м���
	for (auto timeSlice : timeSlices){
		list<list<EdgeCluster*>> newCanadidates = list<list<EdgeCluster*>>();//�µĺ�ѡ���м���
		for (auto canadidate : canadidates){
			EdgeCluster* lastSnapshotCluster = canadidate.back();
			list<EdgeCluster*> assignedEdgeClusters = extendDensityEdges(lastSnapshotCluster);
			if (assignedEdgeClusters.size() == 0){
				if (canadidate.size() >= DE_DURATIVE){//�������������
					resultsList.push_back(canadidate);
				}
			}
			else{
				for (auto assignedEdgeCluster : assignedEdgeClusters){
					newCanadidates.push_back(canadidate);
					newCanadidates.back().push_back(assignedEdgeCluster);
				}
			}
		}
		for (auto edgeCluster : timeSlice->clusters){
			//��ǰʱ��Ƭ�е�·�ξ���δ������չ��������СԪ�ظ�������������Ϊ�µ�һ����ѡ����
			if ((!edgeCluster.second->assigned) && edgeCluster.second->clusterObjects.size() >= DE_MINOBJECTS){
				edgeCluster.second->assigned = true;
				list<EdgeCluster*> densityEdges = list<EdgeCluster*>();
				densityEdges.push_back(edgeCluster.second);
				newCanadidates.push_back(densityEdges);
			}
		}
		canadidates = newCanadidates;//���º�ѡ���м���
	}
	return resultsList;
}

int main(){
	routeNetwork = Map(filePath, 500);//����·��
	edgeCluster();
	return 0;
}