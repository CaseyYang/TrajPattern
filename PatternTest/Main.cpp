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
string mapDirectory = "�¼���·��\\";
string semanticRoadFilePath = "NDBC��չ\\semanticRoad.txt";
string trajInputDirectory = "9daysForTrajPattern\\input";
string matchedEdgeDirectory = "9daysForTrajPattern\\answer";
string semanticRoadNetworkJsonFileName = "RouteNetworkData.js";
Map routeNetwork(rootDirectory + mapDirectory, 500);
vector<NewTimeSlice*> timeSlices;
list<list<EdgeCluster*>> resultsList;//���
int poiSize = 13;
ofstream os, out;

//�Ա�ʵ��׼����������ȡ�켣�ļ�����������������
vector<TimeSlice*> clusterDemo() {
	vector<TimeSlice*> timeSlices = vector<TimeSlice*>(1440);//��ʼ��ʱ��Ƭ����
	int timeStamp = 0;
	for (int timeStamp = 0; timeStamp < 1440; timeStamp++) {
		timeSlices.at(timeStamp) = new TimeSlice(timeStamp);
	}
	scanTrajFolder(rootDirectory + mapDirectory, trajInputDirectory, timeSlices);//����켣
	cout << "�������й켣" << endl;
	int outIndexCount = 0;
	for each (TimeSlice* timeSlice in timeSlices)//�Թ켣�����㽨������
	{
		for each (GeoPoint* point in (*timeSlice).points)
		{
			if (!routeNetwork.insertPoint(point)) {
				outIndexCount++;
			}
		}
	}
	cout << "�Թ켣����������ϣ�" << endl;
	cout << "����" << outIndexCount << "����������������Χ��" << endl;
	for each (TimeSlice* timeSlice in timeSlices)
	{
		timeSlice->clustering(routeNetwork);
	}
	//���ĳ��ʱ��Ƭ�ľ�����
	timeSlices[10 * 60]->clustering(routeNetwork);
	outputDBSCANResult(timeSlices[10 * 60]->clusters);
	return timeSlices;
}

//ʵ��׼����������ȡ��ͼƥ��������ɸ�ʱ��Ƭ��·�ξ���
void edgeCluster() {
	timeSlices = vector<NewTimeSlice*>(1440);//��ʼ��ʱ��Ƭ����
	int timeStamp = 0;
	for (int timeStamp = 0; timeStamp < 1440; timeStamp++) {
		timeSlices.at(timeStamp) = new NewTimeSlice(timeStamp);
	}
	scanMapMatchingResultFolder(rootDirectory + mapDirectory, matchedEdgeDirectory, timeSlices, routeNetwork);//�����ͼƥ�����ļ������ʱ��Ƭ��·�ξ���
	cout << "�������е�ͼƥ����" << endl;
	for (auto timeSlice : timeSlices)
	{
		for (auto edgeCluster : timeSlice->clusters) {
			edgeCluster.second->ascertainPriorCanadidates();
		}
	}
}

//naive�����ĸ����������жϸ���������·�ξ����Ƿ�������չ���������㷵��true�����򷵻�false
bool couldExtendOrNot(set<int> &set1, set<int> &set2, int &intersectionCount) {
	set<int> unionResult = set<int>();
	set_union(set1.begin(), set1.end(), set2.begin(), set2.end(), inserter(unionResult, unionResult.begin()));
	set<int> intersectionResult = set<int>();
	set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(), inserter(intersectionResult, intersectionResult.begin()));
	intersectionCount = static_cast<int>(intersectionResult.size());
	double similarity = (intersectionResult.size() + 0.0) / unionResult.size();
	return similarity >= DE_MINSIMILARITY;
}

//naive�����ĸ�������������һʱ��Ƭ����չ������·�ξ���
list<EdgeCluster*> extendDensityEdges(EdgeCluster* edgeCluster) {
	list<EdgeCluster*> result = list<EdgeCluster*>();
	if (edgeCluster->time < timeSlices.size() - 1) {
		for (auto tmpCluster : timeSlices.at(edgeCluster->time + 1)->clusters)
		{
			//������СԪ�ظ��������Ϳ���չ������
			int useless = 0;
			if (tmpCluster.second->clusterObjects.size() >= DE_MINOBJECTS&&couldExtendOrNot(edgeCluster->clusterObjects, tmpCluster.second->clusterObjects, useless)) {
				tmpCluster.second->assigned = true;
				result.push_back(tmpCluster.second);
			}
		}
	}
	return result;
}

//naive����
list<list<EdgeCluster*>> naiveMethod() {
	resultsList = list<list<EdgeCluster*>>();
	list<list<EdgeCluster*>> canadidates = list<list<EdgeCluster*>>();//��ѡ���м���
	for (auto timeSlice : timeSlices) {
		list<list<EdgeCluster*>> newCanadidates = list<list<EdgeCluster*>>();//�µĺ�ѡ���м���
		for (auto canadidate : canadidates) {
			EdgeCluster* lastSnapshotCluster = canadidate.back();
			list<EdgeCluster*> assignedEdgeClusters = extendDensityEdges(lastSnapshotCluster);
			if (assignedEdgeClusters.size() == 0) {
				if (canadidate.size() >= DE_DURATIVE) {//�������������
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
			//��ǰʱ��Ƭ�е�·�ξ���δ������չ��������СԪ�ظ�������������Ϊ�µ�һ����ѡ����
			if ((!edgeCluster.second->assigned) && edgeCluster.second->clusterObjects.size() >= DE_MINOBJECTS) {
				edgeCluster.second->assigned = true;
				list<EdgeCluster*> densityEdges = list<EdgeCluster*>();
				densityEdges.push_back(edgeCluster.second);
				newCanadidates.push_back(densityEdges);
			}
		}
		canadidates = newCanadidates;//���º�ѡ���м���
	}
	return resultsList;
}

//��kֵ��֦�ķ����ĸ�������������һʱ��Ƭ����չ������·�ξ���
//k��֦��(1-DE_SIMILARITY)*|cluster|>=k
list<EdgeCluster*> extendDensityEdgesWithKPruning(EdgeCluster* edgeCluster) {
	list<EdgeCluster*> result = list<EdgeCluster*>();
	if (edgeCluster->time < timeSlices.size() - 1) {
		for (auto tmpCluster : timeSlices.at(edgeCluster->time + 1)->clusters)
		{
			//���ڴ���չʱ��Ƭ�е�·�ξ��ֻ࣬�в�����k��֦�����ſ��ܳ�Ϊ���е���һ��Ԫ��
			if (tmpCluster.second->k <= tmpCluster.second->clusterObjects.size()*(1 - DE_MINSIMILARITY)) {
				int intersectionCount = 0;
				//������СԪ�ظ��������Ϳ���չ������
				if (tmpCluster.second->clusterObjects.size() >= DE_MINOBJECTS&&couldExtendOrNot(edgeCluster->clusterObjects, tmpCluster.second->clusterObjects, intersectionCount)) {
					tmpCluster.second->assigned = true;
					tmpCluster.second->k += intersectionCount;
					edgeCluster->k += intersectionCount;
					result.push_back(tmpCluster.second);
				}
				if (edgeCluster->k > edgeCluster->clusterObjects.size()*(1 - DE_MINSIMILARITY)) {//���ڵ�ǰ���е����һ�����࣬һ������k��֦����ֹͣ��չ
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

//��kֵ��֦�ķ���
list<list<EdgeCluster*>> methodWithKPruning() {
	resultsList = list<list<EdgeCluster*>>();
	list<list<EdgeCluster*>> canadidates = list<list<EdgeCluster*>>();//��ѡ���м���
	for (auto timeSlice : timeSlices) {
		list<list<EdgeCluster*>> newCanadidates = list<list<EdgeCluster*>>();//�µĺ�ѡ���м���
		for (auto canadidate : canadidates) {
			EdgeCluster* lastSnapshotCluster = canadidate.back();
			lastSnapshotCluster->k = 0;
			list<EdgeCluster*> assignedEdgeClusters = extendDensityEdgesWithKPruning(lastSnapshotCluster);
			if (assignedEdgeClusters.size() == 0) {
				if (canadidate.size() >= DE_DURATIVE) {//�������������
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
			//��ǰʱ��Ƭ�е�·�ξ���δ������չ��������СԪ�ظ�������������Ϊ�µ�һ����ѡ����
			if ((!edgeCluster.second->assigned) && edgeCluster.second->clusterObjects.size() >= DE_MINOBJECTS) {
				edgeCluster.second->assigned = true;
				list<EdgeCluster*> densityEdges = list<EdgeCluster*>();
				densityEdges.push_back(edgeCluster.second);
				newCanadidates.push_back(densityEdges);
			}
		}
		canadidates = newCanadidates;//���º�ѡ���м���
	}
	return resultsList;
}

//���ú���ƥ��·����Ϣ�ķ����ĸ�������������һʱ��Ƭ����չ������·�ξ���
//���ú���ƥ��·����Ϣ����֮ǰ����ö����һʱ��Ƭ�е�����·�ξ��಻ͬ��ֻö�ٵ�ǰ·�ξ����������켣�ĺ���ƥ��·�����ڵ�·�ξ���
list<EdgeCluster*> extendDensityEdgesWithKPruningAndMoreInfo(EdgeCluster* edgeCluster) {
	list<EdgeCluster*> result = list<EdgeCluster*>();
	if (edgeCluster->time < timeSlices.size() - 1) {
		for (auto edge : edgeCluster->priorCanadidates) {//�ʹ�kֵ��֦�ķ�����ȣ�������仰��һ������仰�Ժ���һ����
			EdgeCluster* tmpCluster = timeSlices.at(edgeCluster->time + 1)->clusters.at(edge);
			//���ڴ���չʱ��Ƭ�е�·�ξ��ֻ࣬�в�����k��֦�����ſ��ܳ�Ϊ���е���һ��Ԫ��
			if (tmpCluster->k <= tmpCluster->clusterObjects.size()*(1 - DE_MINSIMILARITY)) {
				int intersectionCount = 0;
				//������СԪ�ظ��������Ϳ���չ������
				if (tmpCluster->clusterObjects.size() >= DE_MINOBJECTS&&couldExtendOrNot(edgeCluster->clusterObjects, tmpCluster->clusterObjects, intersectionCount)) {
					tmpCluster->assigned = true;
					tmpCluster->k += intersectionCount;
					edgeCluster->k += intersectionCount;
					result.push_back(tmpCluster);
				}
				if (edgeCluster->k > edgeCluster->clusterObjects.size()*(1 - DE_MINSIMILARITY)) {//���ڵ�ǰ���е����һ�����࣬һ������k��֦����ֹͣ��չ
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

//���ú���ƥ��·����Ϣ�ķ���
list<list<EdgeCluster*>> methodWithKPruningAndMoreInfo() {
	resultsList = list<list<EdgeCluster*>>();
	list<list<EdgeCluster*>> canadidates = list<list<EdgeCluster*>>();//��ѡ���м���
	for (auto timeSlice : timeSlices) {
		list<list<EdgeCluster*>> newCanadidates = list<list<EdgeCluster*>>();//�µĺ�ѡ���м���
		for (auto canadidate : canadidates) {
			EdgeCluster* lastSnapshotCluster = canadidate.back();
			lastSnapshotCluster->k = 0;
			list<EdgeCluster*> assignedEdgeClusters = extendDensityEdgesWithKPruningAndMoreInfo(lastSnapshotCluster);
			if (assignedEdgeClusters.size() == 0) {
				if (canadidate.size() >= DE_DURATIVE) {//�������������
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
			//��ǰʱ��Ƭ�е�·�ξ���δ������չ��������СԪ�ظ�������������Ϊ�µ�һ����ѡ����
			if ((!edgeCluster.second->assigned) && edgeCluster.second->clusterObjects.size() >= DE_MINOBJECTS) {
				edgeCluster.second->assigned = true;
				list<EdgeCluster*> densityEdges = list<EdgeCluster*>();
				densityEdges.push_back(edgeCluster.second);
				newCanadidates.push_back(densityEdges);
			}
		}
		canadidates = newCanadidates;//���º�ѡ���м���
	}
	return resultsList;
}



//�����
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


//����·����������
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

//���·�������������Ϣ��ָ��js�ļ�
void outputSemanticRouteNetworkToJson(string fileName, set<int>&st)
{
	//���ڵ�
	Json::Value root;
	//���ڵ�����
	root["city"] = Json::Value("Singapore");
	Json::StyledWriter sw;
	for each (Edge* edge in routeNetwork.edges)
		if (edge != NULL && st.find(edge->id) != st.end())
		{
			Json::Value partner;
			//�ӽڵ�����
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
	//������ļ�
	ofstream os;
	os.open(fileName);
	os << "routeNetwork =" << endl;
	os << sw.write(root);
	os.close();
}

//int main() {
//	//����·���������ͼƥ����������·�ξ���
//	edgeCluster();//�����ͼƥ����������·�ξ���
//
//	//�ھ�·������
//	clock_t start, finish;
//	start = clock();
//	resultsList = methodWithKPruningAndMoreInfo();
//	finish = clock();
//	cout << "��ʱ��" << finish - start << "����" << endl;
//
//	//����·������
//	cout << "���õ�" << resultsList.size() << "��ģʽ����" << endl;
//	//getDistinctEdges();
//	filterInvalidEdgeSet();
//	//getDistinctEdges();
//	statisticDistinctEdges();
//
//	//getTimeStatistic();
//	//getAverageSpeed();
//
//	//���·������
//	//outputResults("filteredResults.txt");
//	//ͳ��·�γ��ֵ�Ƶ��������������distinctEdges
//	//�������distinctEdges��Json�ļ�
//	OutputDistinctEdgesToJson(statisticDistinctEdges());
//	system("pause");
//	return 0;
//}
void outputJson(string input)
{
	//���ڵ�
	Json::Value root;
	//���ڵ�����
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
			//�ӽڵ�����
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
	_finddata_t fileInfo;//�ļ���Ϣ
	intptr_t lf;//�ļ����
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
	//����POI�ֲ��ļ������poiNums����
	generateSemanticRoad(routeNetwork, rootDirectory + semanticRoadFilePath);
	openFileFolder("E:\\suhao\\venues\\TrajPattern\\PatternTest\\generateJson\\");
	/*	out.open("cout.txt");
		os.open("count.txt");
		for (int i = 10; i <= 15; i += 1)
		{
			//����·����������
			getGlobalSemanticType(routeNetwork.edges, i);
		}
		os.close(); out.close();*/
		//outputSemanticRouteNetworkToJson(semanticRoadNetworkJsonFileName);
		//���POI������ȷ��ʹ�� 
		//outputSemanticRouteNetworkToPlainText(routeNetwork, "semanticResultNormalized.txt");
}

