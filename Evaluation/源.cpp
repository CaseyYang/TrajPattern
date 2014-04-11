#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include "Map.h"
using namespace std;

Map routeNetwork;
string filePath = "D:\\Document\\MDM Lab\\Data\\�¼��¹켣����\\";

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


list<DensityEdges*> resultLists = list<DensityEdges*>();//���н������
set<Edge*> distinctEdges = set<Edge*>();//���н��·�μ���
list<GeoPoint*> resultPoints = list<GeoPoint*>();//���е㼯

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
	ofstream fout("timeStatistic.txt");
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
	for (auto result : resultLists){
		for (auto resultEdge : result->resultEdges){
			if (distinctEdges.find(resultEdge) == distinctEdges.end()){
				distinctEdges.insert(resultEdge);
			}
		}
	}
	cout << "����" << distinctEdges.size() << "������ͬ�ı�" << endl;
}

//ͳ�ƽ��·�ε�ƽ������
void getAverageSpeed(){
	map<int, double> speedStatistic = map<int, double>();
	vector<int> speedCount = vector<int>();
	for (int i = 0; i < 24; i++){
		speedStatistic.insert(make_pair(i, 0));
		speedCount.push_back(0);
	}
	for (auto result : resultLists){
		double dist = 0;
		if (result->resultEdges.front() != result->resultEdges.back()){
			int startId = result->resultEdges.front()->startNodeId;
			int endId = result->resultEdges.back()->startNodeId;
			list<Edge*> edges = list<Edge*>();
			dist = routeNetwork.shortestPathLength(startId, endId, edges) + result->resultEdges.back()->lengthM;
		}
		else{
			dist = result->resultEdges.front()->lengthM;
		}
		double averageSpeed = dist / ((result->endTime - result->startTime) * 60);
		for (int time = result->startTime / 60; time <= result->endTime / 60; time++){
			speedStatistic.at(time) += averageSpeed;
			speedCount.at(time)++;
		}
	}
	ofstream fout("speedStatistic.txt");
	fout.precision(8);
	for (auto pair : speedStatistic){
		fout << pair.first << "\t" << pair.second / speedCount.at(pair.first) << endl;
	}
	fout.close();
}

//�����н��·��תΪjson�ļ�
void distinctEdgesToJson(){
	ofstream fout("Edges.js");
	fout.precision(11);
	fout << "data = " << endl;
	fout << "{\"city\":\"Singapore\"," << endl;
	fout << "\"edges\":[";
	int edgeIndex = 0;
	for each(Edge* edge in distinctEdges){
		if (edge != NULL){
			if (edgeIndex > 0){
				fout << "," << endl;
			}
			fout << "{\"edgeId\":" << edge->id << ",\"numOfFigures\":" << edge->figure->size() << ",\"figures\":[";
			size_t figureIndex = 0;
			for each (GeoPoint* figPoint in *(edge->figure)){
				fout << "{\"x\":" << figPoint->lon << ",\"y\":" << figPoint->lat << "}";
				if (figureIndex < edge->figure->size() - 1){
					fout << ",";
				}
				figureIndex++;
			}
			fout << "]}";
		}
		edgeIndex++;
	}
	fout << "]," << endl;
	fout << "\"trajPoints\":[";
	size_t trajPointIndex = 0;
	for each (GeoPoint* trajPoint in resultPoints)
	{
		fout << "{\"x\":" << trajPoint->lon << ",\"y\":" << trajPoint->lat << "}";
		if (trajPointIndex < resultPoints.size() - 1){
			fout << ",";
		}
		trajPointIndex++;
	}
	fout << "]}";
	fout.close();
}

//�����н��������תΪjson�ļ�
void pointsToJson(){
	ofstream fout("Points.js");
	fout.precision(11);
	fout << "data = " << endl;
	fout << "{\"city\":\"Singapore\"," << endl;
	if (resultPoints.size() > 0){

	}
	fout.close();
}

int main(){
	routeNetwork = Map(filePath, 500);
	readInResult("result.txt");
	//readInResult("result_day1_withtime.txt");
	//readInResult("result_day2_withtime.txt");
	//readInResult("result_day3_withtime.txt");
	//readInResult("result_day4_M3_483.txt");
	//readInResult("result_day5_M3_483.txt");
	//readInResult("result_day6_M3_479.txt");
	//readInResult("result_day7_M3_462.txt");
	getDistinctEdges();
	//getTimeStatistic();
	//distinctEdgesToJson();
	getAverageSpeed();
}