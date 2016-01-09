#include <iostream>
#include <iterator>
#include <time.h>
#include "ReadInTrajs.h"
#include "../MapLibraries/Map.h"
#include "TimeSlice.h"
#include "NewTimeSlice.h"
#include "Parameters.h"
#include "Evaluation.h"
#include "json/json.h"
//#include "Semantics.h"
using namespace std;

string rootDirectory = "D:\\Document\\MDM Lab\\Data\\";
string mapDirectory = "新加坡路网\\";
string semanticRoadFilePath = "NDBC扩展\\semanticRoad.txt";
string trajInputDirectory = "9daysForTrajPattern\\input";
string matchedEdgeDirectory = "9daysForTrajPattern\\answer";
string trajectoryPath = "matchedEdge.txt";
Map routeNetwork(rootDirectory+mapDirectory, 500);
vector<NewTimeSlice*> timeSlices;
list<list<EdgeCluster*>> resultsList;//结果
double semanticThreshold=0.95;
int MinPts = 60;
ofstream os;
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

double standardDeviation(vector<int>input)
{
	double avg=0,ans=0;
	for (int i = 0; i < input.size(); i++)avg += input[i];
	avg /= input.size();
	for (int i = 0; i < input.size(); i++)
		ans += pow((avg - input[i]),2);
	ans = sqrt(ans / input.size());
	return ans;
}
double standardDeviation(vector<GeoPoint*>input)
{
	double ans = 0; GeoPoint* avg = new GeoPoint(0, 0);
	for (int i = 0; i < input.size(); i++)
	{
		avg->lat += input[i]->lat;
		avg->lon += input[i]->lon;
	}
	avg->lat /= input.size();
	avg->lon /= input.size();
	
	for (int i = 0; i < input.size(); i++)
		ans += pow(GeoPoint::distM(avg,input[i]),2);
	ans = sqrt(ans / input.size());
	return ans;
}


//对比实验准备工作：读取轨迹文件、建立索引及聚类
vector<TimeSlice*> clusterDemo() {
	vector<TimeSlice*> timeSlices = vector<TimeSlice*>(1440);//初始化时间片集合
	int timeStamp = 0;
	for (int timeStamp = 0; timeStamp < 1440; timeStamp++) {
		timeSlices.at(timeStamp) = new TimeSlice(timeStamp);
	}
	scanTrajFolder(rootDirectory + mapDirectory, trajInputDirectory, timeSlices);//读入轨迹
	cout << "读入所有轨迹" << endl;
	int outIndexCount = 0;
	for each (TimeSlice* timeSlice in timeSlices)//对轨迹采样点建立索引
	{
		for each (GeoPoint* point in (*timeSlice).points)
		{
			if (!routeNetwork.insertPoint(point)) {
				outIndexCount++;
			}
		}
	}
	cout << "对轨迹建立索引完毕！" << endl;
	cout << "共有" << outIndexCount << "个采样点在索引范围外" << endl;
	for each (TimeSlice* timeSlice in timeSlices)
	{
		timeSlice->clustering(routeNetwork);
	}
	//输出某个时间片的聚类结果
	timeSlices[10 * 60]->clustering(routeNetwork);
	outputDBSCANResult(timeSlices[10 * 60]->clusters);
	return timeSlices;
}

//实验准备工作：读取地图匹配结果，组成各时间片的路段聚类
void edgeCluster() {
	timeSlices = vector<NewTimeSlice*>(1440);//初始化时间片集合
	int timeStamp = 0;
	for (int timeStamp = 0; timeStamp < 1440; timeStamp++) {
		timeSlices.at(timeStamp) = new NewTimeSlice(timeStamp);
	}
	scanMapMatchingResultFolder(rootDirectory + mapDirectory, matchedEdgeDirectory, timeSlices, routeNetwork);//读入地图匹配结果文件，填充时间片和路段聚类
	cout << "读入所有地图匹配结果" << endl;
	for (auto timeSlice : timeSlices)
	{
		for (auto edgeCluster : timeSlice->clusters) {
			edgeCluster.second->ascertainPriorCanadidates();
		}
	}
}

//实验准备工作，poiNums数组归一化
void poiNumsNormalize(Map&routeNetwork) {
	for each (Edge* edge in routeNetwork.edges)
	{
		if (edge == NULL) continue;
		int count = 0;
		for each(double num in edge->poiNums) {			
			count += static_cast<int>(num);
		}
		if (count == 0) continue;
		for (int i = 0; i < edge->poiNums.size(); ++i) {
			edge->poiNums[i] = edge->poiNums[i] / count;
		}
	}
}

//naive方法的辅助函数：判断给定的两个路段聚类是否满足扩展条件：满足返回true；否则返回false
bool couldExtendOrNot(set<int> &set1, set<int> &set2, int &intersectionCount) {
	set<int> unionResult = set<int>();
	set_union(set1.begin(), set1.end(), set2.begin(), set2.end(), inserter(unionResult, unionResult.begin()));
	set<int> intersectionResult = set<int>();
	set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(), inserter(intersectionResult, intersectionResult.begin()));
	intersectionCount = static_cast<int>(intersectionResult.size());
	double similarity = (intersectionResult.size() + 0.0) / unionResult.size();
	return similarity >= DE_MINSIMILARITY;
}

//naive方法的辅助函数：在下一时间片中扩展给定的路段聚类
list<EdgeCluster*> extendDensityEdges(EdgeCluster* edgeCluster) {
	list<EdgeCluster*> result = list<EdgeCluster*>();
	if (edgeCluster->time < timeSlices.size() - 1) {
		for (auto tmpCluster : timeSlices.at(edgeCluster->time + 1)->clusters)
		{
			//满足最小元素个数条件和可扩展性条件
			int useless = 0;
			if (tmpCluster.second->clusterObjects.size() >= DE_MINOBJECTS&&couldExtendOrNot(edgeCluster->clusterObjects, tmpCluster.second->clusterObjects, useless)) {
				tmpCluster.second->assigned = true;
				result.push_back(tmpCluster.second);
			}
		}
	}
	return result;
}

//naive方法
list<list<EdgeCluster*>> naiveMethod() {
	resultsList = list<list<EdgeCluster*>>();
	list<list<EdgeCluster*>> canadidates = list<list<EdgeCluster*>>();//候选序列集合
	for (auto timeSlice : timeSlices) {
		list<list<EdgeCluster*>> newCanadidates = list<list<EdgeCluster*>>();//新的候选序列集合
		for (auto canadidate : canadidates) {
			EdgeCluster* lastSnapshotCluster = canadidate.back();
			list<EdgeCluster*> assignedEdgeClusters = extendDensityEdges(lastSnapshotCluster);
			if (assignedEdgeClusters.size() == 0) {
				if (canadidate.size() >= DE_DURATIVE) {//满足持续性条件
					resultsList.push_back(canadidate);
				}
			}
			else {
				for (auto assignedEdgeCluster : assignedEdgeClusters) {
					newCanadidates.push_back(canadidate);
					newCanadidates.back().push_back(assignedEdgeCluster);
				}
			}
		}
		for (auto edgeCluster : timeSlice->clusters) {
			//当前时间片中的路段聚类未用于扩展且满足最小元素个数条件，即作为新的一个候选序列
			if ((!edgeCluster.second->assigned) && edgeCluster.second->clusterObjects.size() >= DE_MINOBJECTS) {
				edgeCluster.second->assigned = true;
				list<EdgeCluster*> densityEdges = list<EdgeCluster*>();
				densityEdges.push_back(edgeCluster.second);
				newCanadidates.push_back(densityEdges);
			}
		}
		canadidates = newCanadidates;//更新候选序列集合
	}
	return resultsList;
}

//带k值剪枝的方法的辅助函数：在下一时间片中扩展给定的路段聚类
//k剪枝：(1-DE_SIMILARITY)*|cluster|>=k
list<EdgeCluster*> extendDensityEdgesWithKPruning(EdgeCluster* edgeCluster) {
	list<EdgeCluster*> result = list<EdgeCluster*>();
	if (edgeCluster->time < timeSlices.size() - 1) {
		for (auto tmpCluster : timeSlices.at(edgeCluster->time + 1)->clusters)
		{
			//对于带扩展时间片中的路段聚类，只有不满足k剪枝条件才可能成为序列的下一个元素
			if (tmpCluster.second->k <= tmpCluster.second->clusterObjects.size()*(1 - DE_MINSIMILARITY)) {
				int intersectionCount = 0;
				//满足最小元素个数条件和可扩展性条件
				if (tmpCluster.second->clusterObjects.size() >= DE_MINOBJECTS&&couldExtendOrNot(edgeCluster->clusterObjects, tmpCluster.second->clusterObjects, intersectionCount)) {
					tmpCluster.second->assigned = true;
					tmpCluster.second->k += intersectionCount;
					edgeCluster->k += intersectionCount;
					result.push_back(tmpCluster.second);
				}
				if (edgeCluster->k > edgeCluster->clusterObjects.size()*(1 - DE_MINSIMILARITY)) {//对于当前序列的最后一个聚类，一旦满足k剪枝，则停止扩展
					break;
				}
			}
			else {
				continue;
			}
		}
	}
	return result;
}

//带k值剪枝的方法
list<list<EdgeCluster*>> methodWithKPruning() {
	resultsList = list<list<EdgeCluster*>>();
	list<list<EdgeCluster*>> canadidates = list<list<EdgeCluster*>>();//候选序列集合
	for (auto timeSlice : timeSlices) {
		list<list<EdgeCluster*>> newCanadidates = list<list<EdgeCluster*>>();//新的候选序列集合
		for (auto canadidate : canadidates) {
			EdgeCluster* lastSnapshotCluster = canadidate.back();
			lastSnapshotCluster->k = 0;
			list<EdgeCluster*> assignedEdgeClusters = extendDensityEdgesWithKPruning(lastSnapshotCluster);
			if (assignedEdgeClusters.size() == 0) {
				if (canadidate.size() >= DE_DURATIVE) {//满足持续性条件
					resultsList.push_back(canadidate);
				}
			}
			else {
				for (auto assignedEdgeCluster : assignedEdgeClusters) {
					newCanadidates.push_back(canadidate);
					newCanadidates.back().push_back(assignedEdgeCluster);
				}
			}
		}
		for (auto edgeCluster : timeSlice->clusters) {
			//当前时间片中的路段聚类未用于扩展且满足最小元素个数条件，即作为新的一个候选序列
			if ((!edgeCluster.second->assigned) && edgeCluster.second->clusterObjects.size() >= DE_MINOBJECTS) {
				edgeCluster.second->assigned = true;
				list<EdgeCluster*> densityEdges = list<EdgeCluster*>();
				densityEdges.push_back(edgeCluster.second);
				newCanadidates.push_back(densityEdges);
			}
		}
		canadidates = newCanadidates;//更新候选序列集合
	}
	return resultsList;
}

//利用后续匹配路段信息的方法的辅助函数：在下一时间片中扩展给定的路段聚类
//利用后续匹配路段信息：与之前方法枚举下一时间片中的所有路段聚类不同，只枚举当前路段聚类中所含轨迹的后续匹配路段所在的路段聚类
list<EdgeCluster*> extendDensityEdgesWithKPruningAndMoreInfo(EdgeCluster* edgeCluster) {
	list<EdgeCluster*> result = list<EdgeCluster*>();
	if (edgeCluster->time < timeSlices.size() - 1) {
		for (auto edge : edgeCluster->priorCanadidates) {//和带k值剪枝的方法相比，就是这句话不一样；这句话以后都是一样的
			EdgeCluster* tmpCluster = timeSlices.at(edgeCluster->time + 1)->clusters.at(edge);
			//对于带扩展时间片中的路段聚类，只有不满足k剪枝条件才可能成为序列的下一个元素
			if (tmpCluster->k <= tmpCluster->clusterObjects.size()*(1 - DE_MINSIMILARITY)) {
				int intersectionCount = 0;
				//满足最小元素个数条件和可扩展性条件
				if (tmpCluster->clusterObjects.size() >= DE_MINOBJECTS&&couldExtendOrNot(edgeCluster->clusterObjects, tmpCluster->clusterObjects, intersectionCount)) {
					tmpCluster->assigned = true;
					tmpCluster->k += intersectionCount;
					edgeCluster->k += intersectionCount;
					result.push_back(tmpCluster);
				}
				if (edgeCluster->k > edgeCluster->clusterObjects.size()*(1 - DE_MINSIMILARITY)) {//对于当前序列的最后一个聚类，一旦满足k剪枝，则停止扩展
					break;
				}
			}
			else {
				continue;
			}
		}
	}
	return result;
}

//利用后续匹配路段信息的方法
list<list<EdgeCluster*>> methodWithKPruningAndMoreInfo() {
	resultsList = list<list<EdgeCluster*>>();
	list<list<EdgeCluster*>> canadidates = list<list<EdgeCluster*>>();//候选序列集合
	for (auto timeSlice : timeSlices) {
		list<list<EdgeCluster*>> newCanadidates = list<list<EdgeCluster*>>();//新的候选序列集合
		for (auto canadidate : canadidates) {
			EdgeCluster* lastSnapshotCluster = canadidate.back();
			lastSnapshotCluster->k = 0;
			list<EdgeCluster*> assignedEdgeClusters = extendDensityEdgesWithKPruningAndMoreInfo(lastSnapshotCluster);
			if (assignedEdgeClusters.size() == 0) {
				if (canadidate.size() >= DE_DURATIVE) {//满足持续性条件
					resultsList.push_back(canadidate);
				}
			}
			else {
				for (auto assignedEdgeCluster : assignedEdgeClusters) {
					newCanadidates.push_back(canadidate);
					newCanadidates.back().push_back(assignedEdgeCluster);
				}
			}
		}
		for (auto edgeCluster : timeSlice->clusters) {
			//当前时间片中的路段聚类未用于扩展且满足最小元素个数条件，即作为新的一个候选序列
			if ((!edgeCluster.second->assigned) && edgeCluster.second->clusterObjects.size() >= DE_MINOBJECTS) {
				edgeCluster.second->assigned = true;
				list<EdgeCluster*> densityEdges = list<EdgeCluster*>();
				densityEdges.push_back(edgeCluster.second);
				newCanadidates.push_back(densityEdges);
			}
		}
		canadidates = newCanadidates;//更新候选序列集合
	}
	return resultsList;
}



//检查距离相近及语意相似
bool checkSimilarEdge(Edge* edge1, Edge* edge2, Map&routeNetwork)
{
	if (edge1->poiNums.size() == 0 && edge2->poiNums.size() == 0)return true;
	if (edge1->poiNums.size()==0 || edge2->poiNums.size()==0)return false;
	double t1 = 0, t2 = 0, t3 = 0;
	for (int i = 0; i < min(edge1->poiNums.size(), edge2->poiNums.size()); i++)
	{
		t1 += edge1->poiNums[i] * edge2->poiNums[i];
		t2 += edge1->poiNums[i] * edge1->poiNums[i];
		t3 += edge2->poiNums[i] * edge2->poiNums[i];
	}
	if (t2 < 1e-16 && t3 < 1e-16)return true;
	if (t2 < 1e-16 || t3 < 1e-16)return false;
	if (t1 / sqrt(t2) / sqrt(t3) < semanticThreshold)return false;
	return true;
}

queue<Edge*>regionQuery(Edge* P)
{
	queue<Edge*>NeighborPts;
	while (!NeighborPts.empty())NeighborPts.pop();
	for (auto edge : routeNetwork.edges)
		if (edge != NULL && checkSimilarEdge(P, edge, routeNetwork))
		{
		NeighborPts.push(edge);
		}
	return NeighborPts;

}

//扩展聚类
void expandCluster( queue<Edge*>&NeighborPts, int &count)
{
	while (!NeighborPts.empty())
	{
		Edge*edge = NeighborPts.front(); NeighborPts.pop();
		if (edge->globalSemanticType == -1)
		{
			edge->globalSemanticType = count;
			queue<Edge*>_NeighborPts = regionQuery(edge);
			if (_NeighborPts.size() >= MinPts)
				while (!_NeighborPts.empty())
				{
				Edge*e = _NeighborPts.front(); _NeighborPts.pop();
				if (e->globalSemanticType == -1)NeighborPts.push(e);
				}
		}
	}
}

//计算路段所属种类
void getGlobalSemanticType(Map&routeNetwork)
{
	int count = 0;
	for each (Edge* edge in routeNetwork.edges)
		if (edge != NULL&&edge->globalSemanticType==-1)
			{
				queue<Edge*>NeighborPts = regionQuery(edge);
			//	cout << "NeighborPts.size()=" << NeighborPts.size() << endl;
			//	system("pause");
				if (NeighborPts.size() < MinPts)continue;
				count++;
				expandCluster(NeighborPts, count);
			}

//	os << endl << MinPts << endl;
	os << "count=";
	os <<  count << endl;
	for (int i = -1; i <= count; i++)
	{
		int tot = 0;
		for each (Edge* edge in routeNetwork.edges)
			if (edge && edge->globalSemanticType == i)
				tot++;
		os << i << ' '<<tot<<endl;
	}
	
/*	for each (Edge* edge in routeNetwork.edges)
	{
		cout << edge->id << ' ' << edge->globalSemanticType << endl;
	}
*/
}


void outputJson()
{
	//根节点
	Json::Value root;

	//根节点属性
	root["city"] = Json::Value("Singapore");

	
	Json::StyledWriter sw;
	for each (Edge* edge in routeNetwork.edges)
		if (edge!=NULL)
	{
		Json::Value partner;
		//子节点属性
		partner["edgeId"] = Json::Value(edge->id);
		partner["numOfFigures"] = Json::Value(edge->figure->size());
		for each(auto f in *edge->figure)
		{
			Json::Value figure;
		//	cout << f->lat << ' ' << f->lon << endl;
			figure["x"] = Json::Value(f->lon);
			figure["y"] = Json::Value(f->lat);
			partner["figures"].append(Json::Value(figure));
		}
		partner["localSemanticType"] = Json::Value(edge->globalSemanticType);
	//	cout << sw.write(partner);
		root["edges"].append(Json::Value(partner));
	}
	
	
	
	//输出到文件
	ofstream os;
	os.open("RouteNetworkData.js");
	os << "routeNetwork =" << endl;
	os << sw.write(root);
	os.close();
}

//int main() {
//	//建立路网；读入地图匹配结果并构造路段聚类
//	edgeCluster();//读入地图匹配结果并构造路段聚类
//
//	//挖掘路段序列
//	clock_t start, finish;
//	start = clock();
//	resultsList = methodWithKPruningAndMoreInfo();
//	finish = clock();
//	cout << "用时：" << finish - start << "毫秒" << endl;
//
//	//评估路段序列
//	cout << "共得到" << resultsList.size() << "个模式序列" << endl;
//	//getDistinctEdges();
//	filterInvalidEdgeSet();
//	//getDistinctEdges();
//	statisticDistinctEdges();
//
//	//getTimeStatistic();
//	//getAverageSpeed();
//
//	//输出路段序列
//	//outputResults("filteredResults.txt");
//	//统计路段出现的频数并保存至集合distinctEdges
//	//输出集合distinctEdges至Json文件
//	OutputDistinctEdgesToJson(statisticDistinctEdges());
//	system("pause");
//	return 0;
//}
GeoPoint* getLocation(Edge*edge)
{
	GeoPoint* ans=new GeoPoint(((*(edge->figure->begin()))->lat + (*(edge->figure->rbegin()))->lat )/2, ((*(edge->figure->begin()))->lon +(*(edge->figure->rbegin()))->lon )/2);
	return ans;
}
void readODTrajectory(string inPath)
{
	ifstream fin(inPath);
	int firstTime,time,lastTime, firstEdge, edge; double tmp; bool first = true; char useless;
	while (fin>>time)
	{
		if (time==-1)
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
					GeoPoint* tt = getLocation(routeNetwork.edges[firstEdge]);
					mp[tmp].originSpace.push_back(tt);
					mp[tmp].destSpace.push_back(getLocation(routeNetwork.edges[edge]));
				}
			}
			else first = false;
			fin >> firstTime >> useless >> firstEdge >> useless >> tmp;
			edge = -1;
		}
		else {
			fin >> useless >> edge >> useless >> tmp; lastTime = time;
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
}
bool cmp(const PAIR&lhs, const PAIR&rhs)
{
	return lhs.second.destEdges.size() > rhs.second.destEdges.size();
}
double calcEdge(string fileName)
{
//	cout << fileName << endl;
	int edge; double ans = 0;
	vector<GeoPoint*>input; input.clear();
	ifstream is;
	is.open(fileName);
	while (is >> edge)
	{
		if (edge == -1)
		{
			ans += standardDeviation(input);
			input.clear();
		}
		else
			input.push_back(getLocation(routeNetwork.edges[edge]));
	}
	return ans/2;
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
			ans += standardDeviation(input);
			input.clear();
		}
		else
			input.push_back(edge);
	}
	return ans / 2;
}
void readEdgeTime(string rootPath)
{
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
			cout<<calcEdge(rootPath+inputFileName)<<' ';
			inputFileName.replace(0,5, "time");
			cout << calcTime(rootPath + inputFileName) << endl;
		} while (_findnext(lf, &fileInfo) == 0);
		_findclose(lf);
		return;
	}
}

void main() {
	//读入POI分布文件，填充poiNums数组
	generateSemanticRoad(routeNetwork,rootDirectory + semanticRoadFilePath);
	readEdgeTime("E:\\suhao\\venues\\TrajPattern\\PatternTest\\generateJson\\tmp\\");
/*	readODTrajectory(trajectoryPath);
	vector<PAIR>pairs(mp.begin(), mp.end());
	sort(pairs.begin(), pairs.end(), cmp);
	for (int i = 0; i < 50; i++)
	{
		cout << pairs[i].first.first << ' ' << pairs[i].first.second <<' '<<pairs[i].second.destEdges.size()<<' ';
		cout << (standardDeviation(pairs[i].second.originTime) + standardDeviation(pairs[i].second.destTime))/2 << ' ';
		cout<< (standardDeviation(pairs[i].second.originSpace) + standardDeviation(pairs[i].second.destSpace)) / 2<<endl;
	}*/




	/*
	os.open("count.txt");
	//poiNums数组归一化
	poiNumsNormalize(routeNetwork);
	for (int i = 99; i >90; i-=1)
	{
		semanticThreshold = (double)i*0.01;
		os << endl<<semanticThreshold << '\t';
		for each (Edge* edge in routeNetwork.edges)
			if (edge)edge->globalSemanticType = -1;
		//计算路段所属种类
		getGlobalSemanticType(routeNetwork);
	//	break;
	}
	os.close();*/
	//outputJson();
	//检查POI读入正确性使用 
//	outputSemanticRouteNetwork(routeNetwork, "semanticResultNormalized.txt");
}

