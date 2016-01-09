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
string mapDirectory = "�¼���·��\\";
string semanticRoadFilePath = "NDBC��չ\\semanticRoad.txt";
string trajInputDirectory = "9daysForTrajPattern\\input";
string matchedEdgeDirectory = "9daysForTrajPattern\\answer";
string trajectoryPath = "matchedEdge.txt";
Map routeNetwork(rootDirectory+mapDirectory, 500);
vector<NewTimeSlice*> timeSlices;
list<list<EdgeCluster*>> resultsList;//���
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

//ʵ��׼��������poiNums�����һ��
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



//�������������������
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

//��չ����
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

//����·����������
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
	//���ڵ�
	Json::Value root;

	//���ڵ�����
	root["city"] = Json::Value("Singapore");

	
	Json::StyledWriter sw;
	for each (Edge* edge in routeNetwork.edges)
		if (edge!=NULL)
	{
		Json::Value partner;
		//�ӽڵ�����
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
	
	
	
	//������ļ�
	ofstream os;
	os.open("RouteNetworkData.js");
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
	_finddata_t fileInfo;//�ļ���Ϣ
	intptr_t lf;//�ļ����
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
	//����POI�ֲ��ļ������poiNums����
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
	//poiNums�����һ��
	poiNumsNormalize(routeNetwork);
	for (int i = 99; i >90; i-=1)
	{
		semanticThreshold = (double)i*0.01;
		os << endl<<semanticThreshold << '\t';
		for each (Edge* edge in routeNetwork.edges)
			if (edge)edge->globalSemanticType = -1;
		//����·����������
		getGlobalSemanticType(routeNetwork);
	//	break;
	}
	os.close();*/
	//outputJson();
	//���POI������ȷ��ʹ�� 
//	outputSemanticRouteNetwork(routeNetwork, "semanticResultNormalized.txt");
}

