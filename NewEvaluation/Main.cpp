#include <iostream>
#include <iterator>
#include <time.h>
#include "ReadInTrajs.h"
#include "../MapLibraries/Map.h"
#include "../MapLibraries/GeoPoint.h"
#include "TimeSlice.h"
#include "NewTimeSlice.h"
#include "Parameters.h"
#include "Evaluation.h"
using namespace std;

string rootDirectory = "D:\\Document\\MDM Lab\\Data\\";
string mapDirectory = "新加坡路网\\";
string semanticRoadFilePath = "NDBC扩展\\semanticRoad.txt";
Map routeNetwork(rootDirectory + mapDirectory, 500);
int poiSize = 13;
struct OD {
	set<int>originEdges;
	set<int>destEdges;
	vector<int>originTime;
	vector<int>destTime;
	vector<GeoPoint*>originSpace;
	vector<GeoPoint*>destSpace;
};
typedef pair<pair<int, int>, OD>PAIR;
map<pair<int, int>, OD>mp;

double avgDeviation(vector<int>input)
{
	double ans = 0;
	for (int i = 0; i < input.size(); i++)
		for (int j = i + 1; j < input.size(); j++)
			ans += abs((input[j] - input[i]));
	ans = ans / input.size() / (input.size() - 1);
	return ans;
}
double avgDeviation(vector<GeoPoint*>input)
{
	if (input.size() <= 1)return 0;
	double ans = 0;
	for (int i = 0; i < input.size(); i++)
		for (int j = i + 1; j < input.size(); j++)
			ans += abs(GeoPoint::distM(input[j], input[i]));
	ans = ans / input.size() / (input.size() - 1);
	return ans;
}

GeoPoint* getLocation(Edge*edge)
{
	GeoPoint* ans = new GeoPoint(((*(edge->figure->begin()))->lat + (*(edge->figure->rbegin()))->lat) / 2, ((*(edge->figure->begin()))->lon + (*(edge->figure->rbegin()))->lon) / 2);
	return ans;
}
bool cmp(const PAIR&lhs, const PAIR&rhs)
{
	return lhs.second.destEdges.size() > rhs.second.destEdges.size();
}
void readODTrajectory(string inPath)
{
	ifstream fin(inPath);
	int firstTime, time, lastTime, firstEdge, edge; double tmp; bool first = true; char useless;
	while (fin >> time)
	{
		if (time == -1)
		{
			if (!first)
			{
				if (edge != -1 && firstEdge != -1)
				{
					pair<int, int>tmp = make_pair(routeNetwork.edges[firstEdge]->globalSemanticType, routeNetwork.edges[edge]->globalSemanticType);
					mp[tmp].originEdges.insert(firstEdge);
					mp[tmp].destEdges.insert(edge);
					mp[tmp].originTime.push_back(firstTime);
					mp[tmp].destTime.push_back(lastTime);
					mp[tmp].originSpace.push_back(getLocation(routeNetwork.edges[firstEdge]));
					mp[tmp].destSpace.push_back(getLocation(routeNetwork.edges[edge]));
				}
			}
			else first = false;
			fin >> firstTime >> useless >> firstEdge >> useless >> tmp;
			edge = -1;
		}
		else {
			fin >> useless >> edge >> useless >> tmp;  lastTime = time;
		}
	}
	if (edge != -1 && firstEdge != -1)
	{
		pair<int, int>tmp = make_pair(routeNetwork.edges[firstEdge]->globalSemanticType, routeNetwork.edges[edge]->globalSemanticType);
		mp[tmp].originEdges.insert(firstEdge);
		mp[tmp].destEdges.insert(edge);
		mp[tmp].originTime.push_back(firstTime);
		mp[tmp].destTime.push_back(lastTime);
		mp[tmp].originSpace.push_back(getLocation(routeNetwork.edges[firstEdge]));
		mp[tmp].destSpace.push_back(getLocation(routeNetwork.edges[edge]));
	}
	vector<PAIR>pairs(mp.begin(), mp.end());
	sort(pairs.begin(), pairs.end(), cmp);
	ofstream cout("OD.csv");
	for (int i = 0; i < 20; i++)
	{
		for (int j = 0; j < pairs[i].second.originTime.size(); j++)
			pairs[i].second.originTime[j] /= 60;
		for (int j = 0; j < pairs[i].second.destTime.size(); j++)
			pairs[i].second.destTime[j] /= 60;
		cout << (avgDeviation(pairs[i].second.originSpace) + avgDeviation(pairs[i].second.destSpace)) / 2 << ',';
		cout << (avgDeviation(pairs[i].second.originTime) + avgDeviation(pairs[i].second.destTime)) / 2 << endl;
	}
	cout.close();
}

double calcEdge(string fileName)
{
	int edge; double ans = 0;
	vector<GeoPoint*>input; input.clear();
	ifstream is;
	is.open(fileName);
	while (is >> edge)
	{
		if (edge == -1)
		{
			ans += avgDeviation(input);
			input.clear();
		}
		else
			input.push_back(getLocation(routeNetwork.edges[edge]));
	}
	return ans / 2;
	is.close();
}
double getHHI(vector<Edge*>input)
{
	map<int, int>types;
	for (Edge*edge : input)
		if (types.find(edge->globalSemanticType) == types.end())
			types[edge->globalSemanticType] = 0;
		else
			types[edge->globalSemanticType] += 1;
	double ans = 0;
	for (map<int, int>::iterator it = types.begin(); it != types.end(); it++)
	{
		cout << input.size() << endl;
		ans += pow(double(it->second) / input.size(), 2);
	}
	return ans;
}
double calcSemantic(string fileName)
{
	int edge; double ans = 0;
	vector<Edge*>input; input.clear();
	ifstream is;
	is.open(fileName);
	while (is >> edge)
	{
		if (edge == -1)
		{
			ans += getHHI(input);
			input.clear();
		}
		else
			input.push_back(routeNetwork.edges[edge]);
	}
	return ans / 2;
	is.close();
}
double calcTime(string fileName)
{
	int edge; double ans = 0;
	vector<int>input; input.clear();
	ifstream is(fileName);
	while (is >> edge)
	{
		if (edge == -1)
		{
			ans += avgDeviation(input);
			input.clear();
		}
		else
			input.push_back(edge);
	}
	return ans / 2;
}
void readEdgeTime(string rootPath)
{
	ofstream cout("EdgeTime.csv");
	string completeInputFilesPath = rootPath + "edges*.txt";
	const char* dir = completeInputFilesPath.c_str();
	_finddata_t fileInfo;//文件信息
	intptr_t lf;//文件句柄
	if ((lf = _findfirst(dir, &fileInfo)) == -1l) {
		return;
	}
	else {
		int trajIndex = 0;
		do {
			string inputFileName = fileInfo.name;
			cout << calcEdge(rootPath + inputFileName) << ',';
			inputFileName.replace(0, 5, "time");
			cout << calcTime(rootPath + inputFileName) << endl;
		} while (_findnext(lf, &fileInfo) == 0);
		_findclose(lf);
		return;
	}
	cout.close();
}
void readEdge(string rootPath)
{
	ofstream cout("Edge.csv");
	string completeInputFilesPath = rootPath + "edges*";
	const char* dir = completeInputFilesPath.c_str();
	_finddata_t fileInfo;//文件信息
	intptr_t lf;//文件句柄
	if ((lf = _findfirst(dir, &fileInfo)) == -1l) {
		return;
	}
	else {
		int trajIndex = 0;
		do {
			string inputFileName = fileInfo.name;
			cout << calcEdge(rootPath + inputFileName) << ',';
			cout << calcSemantic(rootPath + inputFileName) << endl;
		} while (_findnext(lf, &fileInfo) == 0);
		_findclose(lf);
		return;
	}
	cout.close();
}

void main() {
	//读入POI分布文件，填充poiNums数组
	generateSemanticRoad(routeNetwork, rootDirectory + semanticRoadFilePath);
	readEdge("D:\\Users\\苏昊\\Documents\\result_10_10_5_5\\result_10_10_5_5\\");
	readEdgeTime("D:\\Users\\苏昊\\Documents\\CGP\\tmp\\");
	readODTrajectory("matchedEdge.txt");
}

