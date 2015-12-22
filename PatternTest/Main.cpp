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
string mapDirectory = "新加坡路网\\";
string semanticRoadFilePath = "NDBC扩展\\semanticRoad.txt";
string trajInputDirectory = "9daysForTrajPattern\\input";
string matchedEdgeDirectory = "day1\\day1_splited_120s_answer";
//string mergedMatchedEdgeFile = "I:\\YangKaixi\\MapMatchingProject\\MapMatchingProject\\Data\\9daysForTrajPattern\\result.txt";//9天
string mergedMatchedEdgeFile = "I:\\YangKaixi\\MapMatchingProject\\MapMatchingProject\\Data\\day1\\20090214_matchedEdge.txt";//1天
string semanticRoadNetworkJsonFileName = "RouteNetworkData.js";
Map routeNetwork(rootDirectory + mapDirectory, 500);
vector<NewTimeSlice*> timeSlices;
list<list<EdgeCluster*>> ndbcResults;			//NDBC结果
list<FineGrainedPattern*> fineGrainedPatterns;	//细粒度轨迹模式
vector<PatternTimeSlot*> patternTimeSlots;		//时间段集合（包含细粒度轨迹模式聚类）
list<CoarseGrainedPattern*> ndbcExtensionResults;


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
	clock_t start, finish;
	start = clock();
	readMergedMapMatchingResult(mergedMatchedEdgeFile, timeSlices, routeNetwork);
	//scanMapMatchingResultFolder(rootDirectory, matchedEdgeDirectory, timeSlices, routeNetwork);//读入地图匹配结果文件，填充时间片和路段聚类
	finish = clock();
	cout << "读入所有地图匹配结果！用时" << finish - start << "毫秒" << endl;
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
	ndbcResults = list<list<EdgeCluster*>>();
	list<list<EdgeCluster*>> canadidates = list<list<EdgeCluster*>>();//候选序列集合
	for (auto timeSlice : timeSlices) {
		list<list<EdgeCluster*>> newCanadidates = list<list<EdgeCluster*>>();//新的候选序列集合
		for (auto canadidate : canadidates) {
			EdgeCluster* lastSnapshotCluster = canadidate.back();
			list<EdgeCluster*> assignedEdgeClusters = extendDensityEdges(lastSnapshotCluster);
			if (assignedEdgeClusters.size() == 0) {
				if (canadidate.size() >= DE_DURATIVE) {//满足持续性条件
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
	return ndbcResults;
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
	ndbcResults = list<list<EdgeCluster*>>();
	list<list<EdgeCluster*>> canadidates = list<list<EdgeCluster*>>();//候选序列集合
	for (auto timeSlice : timeSlices) {
		list<list<EdgeCluster*>> newCanadidates = list<list<EdgeCluster*>>();//新的候选序列集合
		for (auto canadidate : canadidates) {
			EdgeCluster* lastSnapshotCluster = canadidate.back();
			lastSnapshotCluster->k = 0;
			list<EdgeCluster*> assignedEdgeClusters = extendDensityEdgesWithKPruning(lastSnapshotCluster);
			if (assignedEdgeClusters.size() == 0) {
				if (canadidate.size() >= DE_DURATIVE) {//满足持续性条件
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
	return ndbcResults;
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
	ndbcResults = list<list<EdgeCluster*>>();
	list<list<EdgeCluster*>> canadidates = list<list<EdgeCluster*>>();//候选序列集合
	for (auto timeSlice : timeSlices) {
		list<list<EdgeCluster*>> newCanadidates = list<list<EdgeCluster*>>();//新的候选序列集合
		for (auto canadidate : canadidates) {
			EdgeCluster* lastSnapshotCluster = canadidate.back();
			lastSnapshotCluster->k = 0;
			list<EdgeCluster*> assignedEdgeClusters = extendDensityEdgesWithKPruningAndMoreInfo(lastSnapshotCluster);
			if (assignedEdgeClusters.size() == 0) {
				if (canadidate.size() >= DE_DURATIVE) {//满足持续性条件
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
	return ndbcResults;
}

//把NDBC结果包装成FineGrainedPattern数组，便于后续处理
list<FineGrainedPattern*> transferNDBCResultToFineGrainedPatterns() {
	fineGrainedPatterns = list<FineGrainedPattern*>();
	for each (list<EdgeCluster*> pattern in ndbcResults)
	{
		if (pattern.size() == 0) {
			cout << "发现空序列！" << endl;
			system("pause");
			continue;
		}
		FineGrainedPattern* fineGrainedPatternPtr = new FineGrainedPattern(pattern);
		fineGrainedPatterns.push_back(fineGrainedPatternPtr);
	}
	return fineGrainedPatterns;
}

//对时间进行聚类的辅助函数：计算两个元素的相似度
int getSimilarity(int obj1, int obj2) {
	return abs(obj1 - obj2);
}

//对时间进行聚类的辅助函数：分裂现有时间聚类
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

//对细粒度轨迹模式按时间段和语义进行聚类
vector<PatternTimeSlot*> clusterFineGrainedPatterns()
{
	//首先按时间段进行聚类
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
	//然后按语义进行聚类
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

//得到粗粒度轨迹模式的辅助函数：利用Jaccard相似系数判断是否构成粗粒度轨迹模式
bool isCGPonJaccard(set<int> &set1, set<int> &set2) {
	set<int> unionResult = set<int>();
	set_union(set1.begin(), set1.end(), set2.begin(), set2.end(), inserter(unionResult, unionResult.begin()));
	set<int> intersectionResult = set<int>();
	set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(), inserter(intersectionResult, intersectionResult.begin()));
	double similarity = (intersectionResult.size() + 0.0) / unionResult.size();
	return similarity >= CGP_MINSIMILARITY;
}

//得到粗粒度轨迹模式
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
	//读入POI分布文件，填充poiNums数组
	generateSemanticRoad(routeNetwork, rootDirectory + semanticRoadFilePath);
	//检查POI读入正确性使用 
	//outputSemanticRouteNetworkToPlainText(routeNetwork, "semanticResultNormalized.txt");
	//建立路网；读入地图匹配结果并构造路段聚类
	edgeCluster();//读入地图匹配结果并构造路段聚类

	//挖掘路段序列
	clock_t start, finish;
	start = clock();
	//methodWithKPruning();
	ndbcResults = methodWithKPruningAndMoreInfo();
	finish = clock();
	cout << "细粒度轨迹模式挖掘完成！用时：" << finish - start << "毫秒" << endl;

	//NDBC扩展
	transferNDBCResultToFineGrainedPatterns();
	start = clock();
	clusterFineGrainedPatterns();
	getCoarseGrainedPatterns();
	finish = clock();
	cout << "粗粒度轨迹模式挖掘完成！用时：" << finish - start << "毫秒" << endl;
	system("pause");

	////评估路段序列
	//cout << "共得到" << ndbcResults.size() << "个模式序列" << endl;
	////getDistinctEdges();
	//filterInvalidEdgeSet();
	////getDistinctEdges();
	//statisticDistinctEdges();

	////getTimeStatistic();
	////getAverageSpeed();

	////输出路段序列
	////outputResults("filteredResults.txt");
	////统计路段出现的频数并保存至集合distinctEdges
	////输出集合distinctEdges至Json文件
	//OutputDistinctEdgesToJson(statisticDistinctEdges());
	//system("pause");
	//return 0;
}
