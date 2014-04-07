#include "NewTimeSlice.h"

//构造函数，参数为时间片所表示的时间
NewTimeSlice::NewTimeSlice(int time) :time(time){
	sliceObjects = set<int>();
	clusters = map<Edge*, EdgeCluster*>();
}

//向时间片中给定路段添加一条轨迹；若路段聚类已存在则直接添加，否则先创建路段聚类，再添加元素
bool NewTimeSlice::add(int objectId, Edge* edge){
	if (sliceObjects.find(objectId) == sliceObjects.end()){//该时间片中未加入过该条轨迹
		if (clusters.find(edge) == clusters.end()){//该路段聚类尚不存在
			clusters.insert(make_pair(edge, new EdgeCluster(edge, time)));
		}
		if (clusters.at(edge)->add(objectId)){//在路段聚类中插入轨迹成功
			sliceObjects.insert(objectId);
			return true;
		}
		else{
			cout << "同一时间片同一路段出现重复轨迹！" << endl;
			system("pause");
			return false;
		}
	}
	else{
		cout << "同一时间片出现重复轨迹！" << endl;
		system("pause");
		return false;
	}
}