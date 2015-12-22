#include <iostream>
#include <iterator>
#include <time.h>
#include <cmath>
#include <map>
#include "ReadInTrajs.h"
#include "../MapLibraries/Map.h"
#include "TimeSlice.h"
#include "NewTimeSlice.h"
#include "Parameters.h"
#include "Evaluation.h"
#include "../MapLibraries/json/json.h"
#include "FineGrainedPattern.h"
#include "PatternTimeSlot.h"
#include "PatternCluster.h"
#include "CoarseGrainedPattern.h"
using namespace std;

string rootDirectory = "D:\\Document\\MDM Lab\\Data\\";
//string rootDirectory = "I:\\YangKaixi\\MapMatchingProject\\MapMatchingProject\\Data\\";
string mapDirectory = "�¼���·��\\";
string semanticRoadFilePath = "NDBC��չ\\semanticRoad.txt";
string trajInputDirectory = "9daysForTrajPattern\\input";
string matchedEdgeDirectory = "day1\\day1_splited_120s_answer";
//string mergedMatchedEdgeFile = "I:\\YangKaixi\\MapMatchingProject\\MapMatchingProject\\Data\\9daysForTrajPattern\\result.txt";//9��
string mergedMatchedEdgeFile = "I:\\YangKaixi\\MapMatchingProject\\MapMatchingProject\\Data\\day1\\20090214_matchedEdge.txt";//1��
string semanticRoadNetworkJsonFileName = "RouteNetworkData.js";
Map routeNetwork(rootDirectory + mapDirectory, 500);
vector<NewTimeSlice*> timeSlices;
list<list<EdgeCluster*>> ndbcResults;			//NDBC���
list<FineGrainedPattern*> fineGrainedPatterns;	//ϸ���ȹ켣ģʽ
vector<PatternTimeSlot*> patternTimeSlots;		//ʱ��μ��ϣ�����ϸ���ȹ켣ģʽ���ࣩ
list<CoarseGrainedPattern*> ndbcExtensionResults;


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
	clock_t start, finish;
	start = clock();
	readMergedMapMatchingResult(mergedMatchedEdgeFile, timeSlices, routeNetwork);
	//scanMapMatchingResultFolder(rootDirectory, matchedEdgeDirectory, timeSlices, routeNetwork);//�����ͼƥ�����ļ������ʱ��Ƭ��·�ξ���
	finish = clock();
	cout << "�������е�ͼƥ��������ʱ" << finish - start << "����" << endl;
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
	ndbcResults = list<list<EdgeCluster*>>();
	list<list<EdgeCluster*>> canadidates = list<list<EdgeCluster*>>();//��ѡ���м���
	for (auto timeSlice : timeSlices) {
		list<list<EdgeCluster*>> newCanadidates = list<list<EdgeCluster*>>();//�µĺ�ѡ���м���
		for (auto canadidate : canadidates) {
			EdgeCluster* lastSnapshotCluster = canadidate.back();
			list<EdgeCluster*> assignedEdgeClusters = extendDensityEdges(lastSnapshotCluster);
			if (assignedEdgeClusters.size() == 0) {
				if (canadidate.size() >= DE_DURATIVE) {//�������������
					ndbcResults.push_back(canadidate);
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
	return ndbcResults;
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
	ndbcResults = list<list<EdgeCluster*>>();
	list<list<EdgeCluster*>> canadidates = list<list<EdgeCluster*>>();//��ѡ���м���
	for (auto timeSlice : timeSlices) {
		list<list<EdgeCluster*>> newCanadidates = list<list<EdgeCluster*>>();//�µĺ�ѡ���м���
		for (auto canadidate : canadidates) {
			EdgeCluster* lastSnapshotCluster = canadidate.back();
			lastSnapshotCluster->k = 0;
			list<EdgeCluster*> assignedEdgeClusters = extendDensityEdgesWithKPruning(lastSnapshotCluster);
			if (assignedEdgeClusters.size() == 0) {
				if (canadidate.size() >= DE_DURATIVE) {//�������������
					ndbcResults.push_back(canadidate);
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
	return ndbcResults;
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
	ndbcResults = list<list<EdgeCluster*>>();
	list<list<EdgeCluster*>> canadidates = list<list<EdgeCluster*>>();//��ѡ���м���
	for (auto timeSlice : timeSlices) {
		list<list<EdgeCluster*>> newCanadidates = list<list<EdgeCluster*>>();//�µĺ�ѡ���м���
		for (auto canadidate : canadidates) {
			EdgeCluster* lastSnapshotCluster = canadidate.back();
			lastSnapshotCluster->k = 0;
			list<EdgeCluster*> assignedEdgeClusters = extendDensityEdgesWithKPruningAndMoreInfo(lastSnapshotCluster);
			if (assignedEdgeClusters.size() == 0) {
				if (canadidate.size() >= DE_DURATIVE) {//�������������
					ndbcResults.push_back(canadidate);
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
	return ndbcResults;
}

//��NDBC�����װ��FineGrainedPattern���飬���ں�������
list<FineGrainedPattern*> transferNDBCResultToFineGrainedPatterns() {
	fineGrainedPatterns = list<FineGrainedPattern*>();
	for each (list<EdgeCluster*> pattern in ndbcResults)
	{
		if (pattern.size() == 0) {
			cout << "���ֿ����У�" << endl;
			system("pause");
			continue;
		}
		FineGrainedPattern* fineGrainedPatternPtr = new FineGrainedPattern(pattern);
		fineGrainedPatterns.push_back(fineGrainedPatternPtr);
	}
	return fineGrainedPatterns;
}

//��ʱ����о���ĸ�����������������Ԫ�ص����ƶ�
int getSimilarity(int obj1, int obj2) {
	return abs(obj1 - obj2);
}

//��ʱ����о���ĸ�����������������ʱ�����
void splitTimeSlot(vector<PatternTimeSlot*>&timeSlots, int maxj)
{
	ofstream fout("debugSplitTimeSlot.txt");
	int mj;
	double minSSE = 1e10, SSE;
	vector<PatternTimeSlot> a(TIMECLUSTERING_KMEANS_TESTTIME), b(TIMECLUSTERING_KMEANS_TESTTIME);
	int timeSlotCenter1, timeSlotCenter2;
	srand(unsigned(time(NULL)));
	for (int i = 0; i < TIMECLUSTERING_KMEANS_TESTTIME; i++)
	{
		int t1 = rand() % timeSlots[maxj]->timeStamps.size(), t2 = rand() % timeSlots[maxj]->timeStamps.size();
		while (t1 == t2 || getSimilarity(timeSlots[maxj]->timeStamps[t1], timeSlots[maxj]->timeStamps[t2]) < eps) {
			t1 = rand() % timeSlots[maxj]->timeStamps.size();
			t2 = rand() % timeSlots[maxj]->timeStamps.size();
		}
		timeSlotCenter1 = timeSlots[maxj]->timeStamps[t1];
		timeSlotCenter2 = timeSlots[maxj]->timeStamps[t2];
		fout << t1 << " " << timeSlotCenter1 << " " << t2 << " " << timeSlotCenter2 << endl;
		for (int j = 0; j < TIMECLUSTERING_KMEANS_ITERTIME; j++)
		{
			a[i].timeStamps.clear();
			b[i].timeStamps.clear();
			for (int k = 0; k < timeSlots[maxj]->timeStamps.size(); k++) {
				if (getSimilarity(timeSlots[maxj]->timeStamps[k], timeSlotCenter1) < getSimilarity(timeSlots[maxj]->timeStamps[k], timeSlotCenter2)) {
					a[i].insertPattern(timeSlots[maxj]->patterns[k]);
				}
				else {
					b[i].insertPattern(timeSlots[maxj]->patterns[k]);
				}
			}
			timeSlotCenter1 = a[i].center;
			timeSlotCenter2 = b[i].center;
			fout << a[i].timeStamps.size() << " " << a[i].center << " " << b[i].timeStamps.size() << " " << b[i].center << endl;
		}
		fout << endl;
		SSE = a[i].calcSSE() + b[i].calcSSE();
		if (SSE < minSSE) {
			minSSE = SSE;
			mj = i;
		}
	}
	fout.close();
	delete timeSlots[maxj];
	timeSlots[maxj] = new PatternTimeSlot(a[mj]);
	timeSlots.push_back(new PatternTimeSlot(b[mj]));
}

//��ϸ���ȹ켣ģʽ��ʱ��κ�������о���
vector<PatternTimeSlot*> clusterFineGrainedPatterns()
{
	//���Ȱ�ʱ��ν��о���
	patternTimeSlots = vector<PatternTimeSlot*>();
	PatternTimeSlot* initTimeSlot = new PatternTimeSlot(fineGrainedPatterns);
	initTimeSlot->outputTimeStamps("timeStamps.txt");
	patternTimeSlots.push_back(initTimeSlot);
	double maxSSE; int maxj = 0;
	for (int i = 1; i < TIMECLUSTING_KMEANS_K; ++i)
	{
		maxSSE = 0;
		for (int j = 0; j < patternTimeSlots.size(); ++j) {
			if (patternTimeSlots[j]->SSE > maxSSE)
			{
				maxSSE = patternTimeSlots[j]->SSE;
				maxj = j;
			}
		}
		splitTimeSlot(patternTimeSlots, maxj);
	}
	//Ȼ��������о���
	for (int i = 1; i <= TIMECLUSTING_KMEANS_K; ++i) {
		map<int, PatternCluster*> semanticTypePatternClusterMap = map<int, PatternCluster*>();
		for (FineGrainedPattern* pattern : patternTimeSlots[i - 1]->patterns) {
			for each (EdgeCluster* edgeCluster in pattern->edgeClusterPattern)
			{
				PatternCluster* patternCluster = NULL;
				int semanticType = edgeCluster->clusterCoreEdge->globalSemanticType;
				if (semanticTypePatternClusterMap.find(semanticType) == semanticTypePatternClusterMap.end()) {
					patternCluster = new PatternCluster();
					patternCluster->semanticType = semanticType;
					semanticTypePatternClusterMap[semanticType] = patternCluster;
					patternTimeSlots[i - 1]->patternClusters.push_back(patternCluster);
				}
				semanticTypePatternClusterMap[semanticType]->insertPattern(pattern);
			}
		}
	}
	return patternTimeSlots;
}

//�õ������ȹ켣ģʽ�ĸ�������������Jaccard����ϵ���ж��Ƿ񹹳ɴ����ȹ켣ģʽ
bool isCGPonJaccard(set<int> &set1, set<int> &set2) {
	set<int> unionResult = set<int>();
	set_union(set1.begin(), set1.end(), set2.begin(), set2.end(), inserter(unionResult, unionResult.begin()));
	set<int> intersectionResult = set<int>();
	set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(), inserter(intersectionResult, intersectionResult.begin()));
	double similarity = (intersectionResult.size() + 0.0) / unionResult.size();
	return similarity >= CGP_MINSIMILARITY;
}

//�õ������ȹ켣ģʽ
list<CoarseGrainedPattern*> getCoarseGrainedPatterns() {
	ndbcExtensionResults = list<CoarseGrainedPattern*>();
	for (vector<PatternTimeSlot*>::iterator timeSlotIter = patternTimeSlots.begin(); timeSlotIter < patternTimeSlots.end(); ++timeSlotIter) {
		for (vector<PatternTimeSlot*>::iterator nextTimeSloterIter = timeSlotIter + 1; nextTimeSloterIter < patternTimeSlots.end(); ++nextTimeSloterIter) {
			for each (PatternCluster* patternClusterPtr in (*timeSlotIter)->patternClusters)
			{
				for each (PatternCluster* anotherPatternClusterPtr in (*nextTimeSloterIter)->patternClusters)
				{
					if (isCGPonJaccard(patternClusterPtr->objs, anotherPatternClusterPtr->objs)) {
						CoarseGrainedPattern* cgpPtr = new CoarseGrainedPattern();
						cgpPtr->patternClusters.push_back(patternClusterPtr);
						cgpPtr->patternClusters.push_back(anotherPatternClusterPtr);
						ndbcExtensionResults.push_back(cgpPtr);
					}
				}
			}
		}
	}
	return ndbcExtensionResults;
}

int main() {
	//����POI�ֲ��ļ������poiNums����
	generateSemanticRoad(routeNetwork, rootDirectory + semanticRoadFilePath);
	//���POI������ȷ��ʹ�� 
	//outputSemanticRouteNetworkToPlainText(routeNetwork, "semanticResultNormalized.txt");
	//����·���������ͼƥ����������·�ξ���
	edgeCluster();//�����ͼƥ����������·�ξ���

	//�ھ�·������
	clock_t start, finish;
	start = clock();
	//methodWithKPruning();
	ndbcResults = methodWithKPruningAndMoreInfo();
	finish = clock();
	cout << "ϸ���ȹ켣ģʽ�ھ���ɣ���ʱ��" << finish - start << "����" << endl;

	//NDBC��չ
	transferNDBCResultToFineGrainedPatterns();
	start = clock();
	clusterFineGrainedPatterns();
	getCoarseGrainedPatterns();
	finish = clock();
	cout << "�����ȹ켣ģʽ�ھ���ɣ���ʱ��" << finish - start << "����" << endl;
	system("pause");

	////����·������
	//cout << "���õ�" << ndbcResults.size() << "��ģʽ����" << endl;
	////getDistinctEdges();
	//filterInvalidEdgeSet();
	////getDistinctEdges();
	//statisticDistinctEdges();

	////getTimeStatistic();
	////getAverageSpeed();

	////���·������
	////outputResults("filteredResults.txt");
	////ͳ��·�γ��ֵ�Ƶ��������������distinctEdges
	////�������distinctEdges��Json�ļ�
	//OutputDistinctEdgesToJson(statisticDistinctEdges());
	//system("pause");
	//return 0;
}
