#include "Evaluation.h"

int invalidEdges[] = { 55402, 27454, 27489, 55435 };
set<int> invalidEdgeSet = set<int>();
set<Edge*> distinctEdges = set<Edge*>();//���н��·�μ���

//���캯��
SubTraj::SubTraj(int startTime, Edge* edge){
	this->endEdge = edge;
	this->dist = edge->lengthM;
	this->startTime = startTime;
	this->duration = 60;
}

//�����ӹ켣�ĵ�ǰ���һ��·��ָ�룬�����³������Ⱥͳ���ʱ��
void SubTraj::refresh(Edge* edge){
	if (edge != endEdge){
		endEdge = edge;
		dist += edge->lengthM;
	}
	duration += 60;
}

//�����ӹ켣��ƽ���ٶȣ���λΪ��/��
double SubTraj::calculateAverageSpeed(){
	return dist / duration;
}

//����ӹ켣��Ϣ
void SubTraj::outputSubTraj(){
	cout << "�ӹ켣���ȣ�" << this->dist << endl;
	cout << "�ӹ켣���һ��·�γ��ȣ�" << this->endEdge->lengthM << endl;
	cout << "�ӹ켣��ʼʱ�䣺" << this->startTime << endl;
	cout << "�ӹ켣����ʱ�䣺" << this->duration << endl;
}

//������Ҫ���˵�·������
void filterInvalidEdgeSet(){
	for (auto invalidEdge : invalidEdges){
		invalidEdgeSet.insert(invalidEdge);
	}
	for (auto iter = resultsList.begin(); iter != resultsList.end();){
		bool valid = true;
		if (iter->size() > 60){//�����Ĳ�Ҫ
			valid = false;
		}
		else{
			for (auto iter2 = iter->begin(); iter2 != iter->end(); iter2++){//��������������·�ε�Ҳ��Ҫ
				if (invalidEdgeSet.find((*iter2)->clusterCoreEdge->id) != invalidEdgeSet.end()){
					valid = false;
					break;
				}
			}
		}
		if (!valid){
			iter = resultsList.erase(iter);
		}
		else{
			iter++;
		}
	}
	cout << "�������ˣ��õ�" << resultsList.size() << "����Ч����" << endl;
}

//���������м��������а������ӹ켣��ƽ���ٶ�
void getAverageSpeed(){
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
				if (averageSpeed < 28){
					for (int start = subTrajs.at(closedTrajId)->startTime / 60; start <= edgeCluster->time / 60; start++){
						perHourAverageSpeed.at(start) += averageSpeed;
						perHourAverageSpeedCount.at(start)++;
					}
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
			if (averageSpeed < 28){
				for (int start = pair.second->startTime / 60; start <= (result.back()->time + 1) / 60; start++){
					perHourAverageSpeed.at(start) += averageSpeed;
					perHourAverageSpeedCount.at(start)++;
				}
			}
			delete pair.second;
		}
		subTrajs.clear();
		lastTrajs.clear();
	}
	for (auto pair : perHourAverageSpeed){
		if (perHourAverageSpeedCount.at(pair.first) > 0){
			perHourAverageSpeedResults.at(pair.first) = pair.second / perHourAverageSpeedCount.at(pair.first);
		}
		else{
			perHourAverageSpeedResults.at(pair.first) = 0;
		}
	}
	ofstream fout("speedStatistic.txt");
	fout.precision(8);
	for (size_t i = 0; i < perHourAverageSpeedResults.size(); i++){
		fout << i << "\t" << perHourAverageSpeedResults.at(i)*3.6 << endl;
	}
	fout.close();
}

//ͳ�ƽ��·������
void getDistinctEdges(){
	for (auto result : resultsList){
		for (auto resultEdge : result){
			if (distinctEdges.find(resultEdge->clusterCoreEdge) == distinctEdges.end()){
				distinctEdges.insert(resultEdge->clusterCoreEdge);
			}
		}
	}
	cout << "����" << distinctEdges.size() << "������ͬ�ı�" << endl;
	ofstream fout("distinctEdges.txt");
	fout << distinctEdges.size() << endl;
	for (auto edge : distinctEdges){
		fout << edge->id << endl;
	}
	fout.close();
}

//ͳ�ƽ��ʱ��ֲ�
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

//���������м���
void outputResults(string fileName){
	ofstream fout(fileName);
	fout << resultsList.size() << endl;
	for (auto resultList : resultsList){
		//cout << "�������" << resultList.size() << "�����" << endl;
		fout << resultList.size() << ":" << resultList.front()->time << "~" << resultList.back()->time << " ";
		int lastId = -1;
		for (auto resultEdge : resultList){
			int id = resultEdge->clusterCoreEdge->id;
			if (lastId != id){
				fout << id << ",";
			}
			lastId = id;
		}
		fout << endl;
	}
	fout.close();
}