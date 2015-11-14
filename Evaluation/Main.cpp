#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include "Map.h"
using namespace std;

Map routeNetwork;
string filePath = "E:\\Document\\Subjects\\Computer\\Data\\新加坡路网及轨迹数据\\";
int invalidEdges[] = { 55402, 27454, 27489, 55435 };
set<int> invalidEdgeSet = set<int>();


//序列结果
struct DensityEdges {
	int startTime;
	int endTime;
	int resultId;
	list<Edge*> resultEdges;
	DensityEdges(int start, int end, int resultId) {
		this->startTime = start;
		this->endTime = end;
		resultEdges = list<Edge*>();
		this->resultId = resultId;
	}
	void add(Edge* edge) {
		resultEdges.push_back(edge);
	}
};


list<DensityEdges*> resultLists = list<DensityEdges*>();//序列结果集合
set<Edge*> distinctEdges = set<Edge*>();//序列结果路段集合
list<GeoPoint*> resultPoints = list<GeoPoint*>();//序列点集

//读入序列结果文件
void readInResult(string resultFilePath, int resultId) {
	ifstream fin(resultFilePath);
	int totalCount = 0;
	fin >> totalCount;
	int index = 0, validIndex = 0;
	while (index < totalCount) {
		int count = 0, startTime = -1, endTime = -1;
		char useless;
		fin >> count >> useless >> startTime >> useless >> endTime;
		DensityEdges* result = new DensityEdges(startTime, endTime, resultId);
		int edgeIndex;
		int index2 = 0;
		bool valid = true;
		while (index2 < count) {
			fin >> edgeIndex >> useless;
			if (valid&&invalidEdgeSet.find(edgeIndex) == invalidEdgeSet.end()) {
				result->add(routeNetwork.edges.at(edgeIndex));
			}
			else {
				valid = false;
			}
			index2++;
		}
		if (valid) {
			resultLists.push_back(result);
			validIndex++;
		}
		else
		{
			delete result;
		}
		index++;
	}
	fin.close();
	cout << "共得到" << validIndex << "条有效结果序列" << endl;
}

//统计结果时间分布
void getTimeStatistic() {
	map<int, int> timeStatistic = map<int, int>();
	for (int i = 0; i < 24; i++) {
		timeStatistic.insert(make_pair(i, 0));
	}
	ofstream fout("timeStatistic.txt");
	for (auto result : resultLists) {
		for (int time = result->startTime / 60; time <= result->endTime / 60; time++) {
			timeStatistic.at(time)++;
		}
	}
	for (auto pair : timeStatistic) {
		fout << pair.first << "\t" << pair.second << endl;
	}
	fout.close();
}

//统计结果路段数量
void getDistinctEdges() {
	for (auto result : resultLists) {
		for (auto resultEdge : result->resultEdges) {
			if (distinctEdges.find(resultEdge) == distinctEdges.end()) {
				distinctEdges.insert(resultEdge);
			}
		}
	}
	cout << "共有" << distinctEdges.size() << "条不相同的边" << endl;
}

//统计结果路段的平均车速
void getAverageSpeed() {
	map<int, double> speedStatistic = map<int, double>();
	vector<int> speedCount = vector<int>();
	for (int i = 0; i < 24; i++) {
		speedStatistic.insert(make_pair(i, 0));
		speedCount.push_back(0);
	}
	for (auto result : resultLists) {
		double dist = 0;
		Edge* lastResultEdge = NULL;
		for (auto resultEdge : result->resultEdges) {
			if (resultEdge != lastResultEdge) {
				dist += resultEdge->lengthM;
			}
			lastResultEdge = resultEdge;
		}
		double averageSpeed = (dist / ((result->endTime - result->startTime) * 60))*3.6;
		for (int time = result->startTime / 60; time <= result->endTime / 60; time++) {
			speedStatistic.at(time) += averageSpeed;
			speedCount.at(time)++;
		}
	}
	ofstream fout("speedStatistic.txt");
	fout.precision(8);
	for (auto pair : speedStatistic) {
		if (speedCount.at(pair.first) > 0) {
			fout << pair.first << "\t" << pair.second / speedCount.at(pair.first) << "\t" << speedCount.at(pair.first) << endl;
		}
		else {
			fout << pair.first << "\t0\t0" << endl;
		}
	}
	fout.close();
}

//把序列结果路段转为json文件
void distinctEdgesToJson() {
	ofstream fout("Edges.js");
	fout.precision(11);
	fout << "data = " << endl;
	fout << "{\"city\":\"Singapore\"," << endl;
	fout << "\"edges\":[";
	int edgeIndex = 0;
	for each(Edge* edge in distinctEdges) {
		if (edge != NULL) {
			if (edgeIndex > 0) {
				fout << "," << endl;
			}
			fout << "{\"edgeId\":" << edge->id << ",\"numOfFigures\":" << edge->figure->size() << ",\"figures\":[";
			size_t figureIndex = 0;
			for each (GeoPoint* figPoint in *(edge->figure)) {
				fout << "{\"x\":" << figPoint->lon << ",\"y\":" << figPoint->lat << "}";
				if (figureIndex < edge->figure->size() - 1) {
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
		if (trajPointIndex < resultPoints.size() - 1) {
			fout << ",";
		}
		trajPointIndex++;
	}
	fout << "]}";
	fout.close();
}

//把序列结果采样点转为json文件
void pointsToJson() {
	ofstream fout("Points.js");
	fout.precision(11);
	fout << "data = " << endl;
	fout << "{\"city\":\"Singapore\"," << endl;
	if (resultPoints.size() > 0) {

	}
	fout.close();
}

int main() {
	routeNetwork = Map(filePath, 500);
	for (auto invalidEdge : invalidEdges) {
		invalidEdgeSet.insert(invalidEdge);
	}
	readInResult("result_day1_withtime.txt", 1);
	readInResult("result_day2_withtime.txt", 2);
	readInResult("result_day3_withtime.txt", 3);
	readInResult("result_day4_M3_483.txt", 4);
	readInResult("result_day5_M3_483.txt", 5);
	readInResult("result_day6_M3_479.txt", 6);
	readInResult("result_day7_M3_462.txt", 7);
	getDistinctEdges();
	//getTimeStatistic();
	//distinctEdgesToJson();
	getAverageSpeed();
}