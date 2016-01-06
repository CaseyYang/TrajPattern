#include <iostream>
#include <iterator>
#include <time.h>
#include "ReadInTrajs.h"
#include "../MapLibraries/Map.h"
#include "TimeSlice.h"
#include "NewTimeSlice.h"
#include "Parameters.h"
#include "Evaluation.h"
#include "../MapLibraries/json/json.h"
//#include "Semantics.h"
using namespace std;

string rootDirectory = "D:\\Document\\MDM Lab\\Data\\";
string mapDirectory = "新加坡路网\\";
string semanticRoadFilePath = "NDBC扩展\\semanticRoad.txt";
string trajInputDirectory = "9daysForTrajPattern\\input";
string matchedEdgeDirectory = "9daysForTrajPattern\\answer";
string semanticRoadNetworkJsonFileName = "RouteNetworkData.js";
Map routeNetwork(rootDirectory + mapDirectory, 500);
vector<NewTimeSlice*> timeSlices;
list<list<EdgeCluster*>> resultsList;//结果
int poiSize = 13;
ofstream os, out;

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



//求距离
double getDistance(Edge* edge1, Edge* edge2)
{
	double t1 = 0, t2 = 0, t3 = 0;
	for (int i = 0; i < poiSize; i++)
	{
		t1 += edge1->poiNums[i] * edge2->poiNums[i];
		t2 += edge1->poiNums[i] * edge1->poiNums[i];
		t3 += edge2->poiNums[i] * edge2->poiNums[i];
	}
	return 1 - t1 / sqrt(t2) / sqrt(t3);
}

Edge* calcCenter(vector<Edge*>& edges)
{
	Edge* center = new Edge();
	center->poiNums = vector<double>(poiSize, 0);
	for (int i = 0; i < poiSize; i++)
	{
		for (auto edge : edges)
			center->poiNums[i] += edge->poiNums[i];
		center->poiNums[i] /= edges.size();
	}
	double t = 0;
	for (int i = 0; i < poiSize; i++)
		t += center->poiNums[i] * center->poiNums[i];
	t = sqrt(t);
	for (int i = 0; i < poiSize; i++)
		center->poiNums[i] /= t;
	return center;
}

double calcSSE(semanticCluster& cluster, Edge*center)
{

	double SSE = 0;
	for (auto edge : cluster.cluster)
	{
		SSE += getDistance(edge, center);
	}
	cluster.SSE = SSE;
	return SSE;
}

void splitCluster(vector<semanticCluster>&clusters, int maxj)
{
	int iterTime = 15, testTime = 20, mj;
	double minSSE = 1e10, SSE;
	vector<semanticCluster>a(testTime), b(testTime);
	Edge*center1, *center2;
	srand(unsigned(time(NULL)));
	for (int i = 0; i < testTime; i++)
	{
		int t1 = rand() % clusters[maxj].cluster.size(), t2 = rand() % clusters[maxj].cluster.size();
		while (t1 == t2 || getDistance(clusters[maxj].cluster[t1], clusters[maxj].cluster[t2]) < 1e-10)t2 = rand() % clusters[maxj].cluster.size();
		center1 = clusters[maxj].cluster[t1]; center2 = clusters[maxj].cluster[t2];
		for (int j = 0; j < iterTime; j++)
		{
			a[i].cluster.clear(); b[i].cluster.clear();
			for (int k = 0; k < clusters[maxj].cluster.size(); k++)
				if (getDistance(clusters[maxj].cluster[k], center1) < getDistance(clusters[maxj].cluster[k], center2))
					a[i].cluster.push_back(clusters[maxj].cluster[k]);
				else b[i].cluster.push_back(clusters[maxj].cluster[k]);
				center1 = calcCenter(a[i].cluster); center2 = calcCenter(b[i].cluster);
				out << a[i].cluster.size() << ' ' << b[i].cluster.size() << ' ' << calcSSE(a[i], center1) << ' ' << calcSSE(b[i], center2) << endl;
		}
		out << endl;
		SSE = calcSSE(a[i], center1) + calcSSE(b[i], center2);
		if (SSE < minSSE) { minSSE = SSE; mj = i; }
	}
	clusters[maxj] = a[mj]; clusters.push_back(b[mj]);
}


//计算路段所属种类
void getGlobalSemanticType(vector<Edge*> &edges, int k)
{
	vector<semanticCluster>clusters;
	semanticCluster cluster;
	for each (Edge* edge in edges)
		if (edge&&edge->poiNums.size() >= poiSize)
			cluster.cluster.push_back(edge);
	clusters.push_back(cluster);
	double  maxSSE; int maxj = 0;
	for (int i = 1; i < k; i++)
	{
		maxSSE = 0;
		for (int j = 0; j < clusters.size(); j++)
			if (clusters[j].SSE>maxSSE)
			{
				maxSSE = clusters[j].SSE; maxj = j;
			}
		//cout << maxSSE << endl;
		splitCluster(clusters, maxj);

	}
	for (int i = 1; i <= k; i++)
		for (auto edge : clusters[i - 1].cluster)
			edge->globalSemanticType = i;
	os << "count=" << k << endl;
	cout << "count=" << k << endl;
	for (int i = -1; i <= k; i++)
	{
		int tot = 0;
		for each (Edge* edge in routeNetwork.edges)
			if (edge && edge->globalSemanticType == i)
				tot++;
		os << i << ' ' << tot << ' ';
		//	cout << i << ' ' << tot << ' ';
		//	if (i > 0)os << clusters[i - 1].SSE;
		os << endl; //cout << endl;
	}

	/*	for each (Edge* edge in routeNetwork.edges)
		{
			cout << edge->id << ' ' << edge->globalSemanticType << endl;
		}
	*/
}

//输出路网及语义聚类信息至指定js文件
void outputSemanticRouteNetworkToJson(string fileName, set<int>&st)
{
	//根节点
	Json::Value root;
	//根节点属性
	root["city"] = Json::Value("Singapore");
	Json::StyledWriter sw;
	for each (Edge* edge in routeNetwork.edges)
		if (edge != NULL && st.find(edge->id) != st.end())
		{
			Json::Value partner;
			//子节点属性
			partner["edgeId"] = Json::Value(edge->id);
			partner["numOfFigures"] = Json::Value(edge->figure->size());
			for each(auto f in *edge->figure)
			{
				Json::Value figure;
				//cout << f->lat << ' ' << f->lon << endl;
				figure["x"] = Json::Value(f->lon);
				figure["y"] = Json::Value(f->lat);
				partner["figures"].append(Json::Value(figure));
			}
			partner["localSemanticType"] = Json::Value(edge->globalSemanticType);
			//cout << sw.write(partner);
			root["edges"].append(Json::Value(partner));
		}
	//输出到文件
	ofstream os;
	os.open(fileName);
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
void outputJson(string input)
{
	//根节点
	Json::Value root;
	//根节点属性
	root["city"] = Json::Value("Singapore");
	Json::StyledWriter sw;
	ifstream is;
	is.open(input);
	int edge, type = 1;
	set<int>st;
	string output = input.replace(input.size() - 3, 3, "js");
	while (is >> edge)
	{
		if (edge == -1)type = 2;
		else {
			Json::Value partner;
			//子节点属性
			partner["edgeId"] = Json::Value(routeNetwork.edges[edge]->id);
			partner["numOfFigures"] = Json::Value(routeNetwork.edges[edge]->figure->size());
			for each(auto f in *routeNetwork.edges[edge]->figure)
			{
				Json::Value figure;
				//cout << f->lat << ' ' << f->lon << endl;
				figure["x"] = Json::Value(f->lon);
				figure["y"] = Json::Value(f->lat);
				partner["figures"].append(Json::Value(figure));
			}
			partner["localSemanticType"] = Json::Value(type);
			//cout << sw.write(partner);
			root["edges"].append(Json::Value(partner));
			
		}
	}
	ofstream os;
	os.open(output);
	os << "routeNetwork =" << endl;
	os << sw.write(root);
	os.close();

}
void  openFileFolder(string rootPath)
{
	string completeInputFilesPath = rootPath+"*.txt";
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
			outputJson(rootPath+inputFileName);
		} while (_findnext(lf, &fileInfo) == 0);
		_findclose(lf);
		return;
	}

}
void main() {
	//读入POI分布文件，填充poiNums数组
	generateSemanticRoad(routeNetwork, rootDirectory + semanticRoadFilePath);
	openFileFolder("E:\\suhao\\venues\\TrajPattern\\PatternTest\\generateJson\\");
	/*	out.open("cout.txt");
		os.open("count.txt");
		for (int i = 10; i <= 15; i += 1)
		{
			//计算路段所属种类
			getGlobalSemanticType(routeNetwork.edges, i);
		}
		os.close(); out.close();*/
		//outputSemanticRouteNetworkToJson(semanticRoadNetworkJsonFileName);
		//检查POI读入正确性使用 
		//outputSemanticRouteNetworkToPlainText(routeNetwork, "semanticResultNormalized.txt");
}

