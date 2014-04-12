#include "Evaluation.h"

//构造函数
SubTraj::SubTraj(int startTime, Edge* edge){
	this->endEdge = edge;
	this->dist = edge->lengthM;
	this->startTime = startTime;
	this->duration = 60;
}

//更新子轨迹的当前最后一个路段指针，并更新持续长度和持续时间
void SubTraj::refresh(Edge* edge){
	if (edge != endEdge){
		endEdge = edge;
		dist += edge->lengthM;
	}
	duration += 60;
}

//返回子轨迹的平均速度，单位为米/秒
double SubTraj::calculateAverageSpeed(){
	return dist / duration;
}

//构造需要过滤的路段序列
void filterInvalidEdgeSet(){
	for (auto invalidEdge : invalidEdges){
		invalidEdgeSet.insert(invalidEdge);
	}
	for (auto iter = resultsList.begin(); iter != resultsList.end(); iter++){
		bool valid = true;
		for (auto iter2 = iter->begin(); iter2 != iter->end(); iter2++){
			if (invalidEdgeSet.find((*iter2)->clusterCoreEdge->id) != invalidEdgeSet.end()){
				valid = false;
				break;
			}
		}
		if (!valid){
			resultsList.remove(*iter);
		}
	}
}

//计算结果序列集合中所有包含的子轨迹的平均速度
vector<double> getAverageSpeed(){
	vector<double> perHourAverageSpeedResults = vector<double>();
	map<int, double> perHourAverageSpeed = map<int, double>();
	vector<int> perHourAverageSpeedCount = vector<int>();
	for (int i = 0; i < 24; i++){
		perHourAverageSpeed.insert(make_pair(i, 0));
		perHourAverageSpeedCount.push_back(0);
		perHourAverageSpeedResults.push_back(0);
	}
	for (auto result : resultsList){
		map<int, SubTraj*> subTrajs = map<int, SubTraj*>();
		set<int> lastTrajs = set<int>();
		for (auto edgeCluster : result){
			set<int> closed = set<int>();
			set_difference(lastTrajs.begin(), lastTrajs.end(), edgeCluster->clusterObjects.begin(), edgeCluster->clusterObjects.end(), inserter(closed, closed.begin()));
			for (auto closedTrajId : closed){
				double averageSpeed = subTrajs.at(closedTrajId)->calculateAverageSpeed();
				for (int start = subTrajs.at(closedTrajId)->startTime / 60; start <= edgeCluster->time / 60; start++){
					perHourAverageSpeed.at(start) += averageSpeed;
					perHourAverageSpeedCount.at(start)++;
				}
				delete subTrajs.at(closedTrajId);
				subTrajs.erase(closedTrajId);
			}
			for (auto object : edgeCluster->clusterObjects){
				if (subTrajs.find(object) == subTrajs.end()){
					subTrajs.insert(make_pair(object, new SubTraj(edgeCluster->time, edgeCluster->clusterCoreEdge)));
				}
				else{
					subTrajs.at(object)->refresh(edgeCluster->clusterCoreEdge);
				}
			}
			lastTrajs.clear();
			for (auto pair : subTrajs){
				lastTrajs.insert(pair.first);
			}
		}
		for (auto pair : subTrajs)
		{
			double averageSpeed = pair.second->calculateAverageSpeed();
			for (int start = pair.second->startTime / 60; start <= (result.back()->time + 1) / 60; start++){
				perHourAverageSpeed.at(start) += averageSpeed;
				perHourAverageSpeedCount.at(start)++;
			}
			delete pair.second;
		}
		subTrajs.clear();
		lastTrajs.clear();
	}
	for (auto pair : perHourAverageSpeed){
		perHourAverageSpeedResults.at(pair.first) = pair.second / perHourAverageSpeedCount.at(pair.first);
	}
	return perHourAverageSpeedResults;
}

//统计结果路段数量
void getDistinctEdges(){
	for (auto result : resultsList){
		for (auto resultEdge : result){
			if (distinctEdges.find(resultEdge->clusterCoreEdge) == distinctEdges.end()){
				distinctEdges.insert(resultEdge->clusterCoreEdge);
			}
		}
	}
	cout << "共有" << distinctEdges.size() << "条不相同的边" << endl;
}

//统计结果时间分布
void getTimeStatistic(){
	map<int, int> timeStatistic = map<int, int>();
	for (int i = 0; i < 24; i++){
		timeStatistic.insert(make_pair(i, 0));
	}
	ofstream fout("timeStatistic.txt");
	for (auto result : resultsList){
		for (int time = result.front()->time / 60; time <= result.back()->time / 60; time++){
			timeStatistic.at(time)++;
		}
	}
	for (auto pair : timeStatistic){
		fout << pair.first << "\t" << pair.second << endl;
	}
	fout.close();
}