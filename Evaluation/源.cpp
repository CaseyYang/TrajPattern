#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include "Map.h"
using namespace std;

Map routeNetwork;
string filePath = "D:\\Document\\Subjects\\Computer\\Develop\\Data\\SingaporeData\\";

//���н��
struct DensityEdges{
	int startTime;
	int endTime;
	list<Edge*> resultEdges;
	DensityEdges(int start, int end){
		this->startTime = start;
		this->endTime = end;
		resultEdges = list<Edge*>();
	}
	void add(Edge* edge){
		resultEdges.push_back(edge);
	}
};

//���н������
list<DensityEdges*> resultLists = list<DensityEdges*>();

//�������н���ļ�
void readInResult(string resultFilePath){
	ifstream fin(resultFilePath);
	int totalCount = 0;
	fin >> totalCount;
	int index = 0;
	while (index < totalCount){
		int count = 0, startTime = -1, endTime = -1;
		char useless;
		fin >> count >> useless >> startTime >> useless >> endTime;
		DensityEdges* result = new DensityEdges(startTime, endTime);
		//cout << startTime << " " << endTime<<endl;
		//system("pause");
		int edgeIndex;
		int index2 = 0;
		while (index2 < count){
			fin >> edgeIndex >> useless;
			result->add(routeNetwork.edges.at(edgeIndex));
			index2++;
		}
		resultLists.push_back(result);
		index++;
	}
	fin.close();
}

//ͳ�ƽ��ʱ��ֲ�
void getTimeStatistic(){
	map<int, int> timeStatistic = map<int, int>();
	for (int i = 0; i < 24; i++){
		timeStatistic.insert(make_pair(i, 0));
	}
	ofstream fout("statistic.txt");
	for (auto result : resultLists){
		for (int time = result->startTime / 60; time <= result->endTime / 60; time++){
			timeStatistic.at(time)++;
		}
	}
	for (auto pair : timeStatistic){
		fout << pair.first << "\t" << pair.second << endl;
	}
	fout.close();
}

//ͳ�ƽ��·������
void getDistinctEdges(){
	set<Edge*> distinctEdges = set<Edge*>();
	for (auto result : resultLists){
		for (auto resultEdge:result->resultEdges){
			if (distinctEdges.find(resultEdge) == distinctEdges.end()){
				distinctEdges.insert(resultEdge);
			}
		}
	}
	cout << "����" << distinctEdges.size() << "������ͬ�ı�" << endl;
	for (auto edge : distinctEdges){
		cout << edge->id << " ";
	}
	cout << endl;
}

int main(){
	routeNetwork = Map(filePath, 500);
	readInResult("result_day1_withtime.txt");
	//getTimeStatistic();
	getDistinctEdges();
}