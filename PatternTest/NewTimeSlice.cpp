#include "NewTimeSlice.h"

//���캯��������Ϊʱ��Ƭ����ʾ��ʱ��
NewTimeSlice::NewTimeSlice(int time) :time(time){
	sliceObjects = set<int>();
	clusters = map<Edge*, EdgeCluster*>();
}

//��ʱ��Ƭ�и���·�����һ���켣����·�ξ����Ѵ�����ֱ����ӣ������ȴ���·�ξ��࣬�����Ԫ��
bool NewTimeSlice::add(int objectId, Edge* edge){
	if (sliceObjects.find(objectId) == sliceObjects.end()){//��ʱ��Ƭ��δ����������켣
		if (clusters.find(edge) == clusters.end()){//��·�ξ����в�����
			clusters.insert(make_pair(edge, new EdgeCluster(edge, time)));
		}
		if (clusters.at(edge)->add(objectId)){//��·�ξ����в���켣�ɹ�
			sliceObjects.insert(objectId);
			return true;
		}
		else{
			cout << "ͬһʱ��Ƭͬһ·�γ����ظ��켣��" << endl;
			system("pause");
			return false;
		}
	}
	else{
		cout << "ͬһʱ��Ƭ�����ظ��켣��" << endl;
		system("pause");
		return false;
	}
}