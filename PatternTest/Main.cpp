#include <iostream>
#include <iterator>
#include <time.h>
#include "ReadInTrajs.h"
#include "Map.h"
#include "TimeSlice.h"
#include "NewTimeSlice.h"
#include "Parameters.h"
#include "Evaluation.h"
using namespace std;

string filePath = "E:\\MapMatchingProject\\Data\\新加坡数据\\";
string inputDirectory = "9daysForTrajPattern\\input";
string answerDirectory = "9daysForTrajPattern\\answer";
Map routeNetwork(filePath, 500);
vector<NewTimeSlice*> timeSlices;
list<list<EdgeCluster*>> resultsList;//结果

//对比实验准备工作：读取轨迹文件、建立索引及聚类
vector<TimeSlice*> clusterDemo(){
	vector<TimeSlice*> timeSlices = vector<TimeSlice*>(1440);//初始化时间片集合
	int timeStamp = 0;
	for (int timeStamp = 0; timeStamp < 1440; timeStamp++){
		timeSlices.at(timeStamp) = new TimeSlice(timeStamp);
	}
	scanTrajFolder(filePath, inputDirectory, timeSlices);//读入轨迹
	cout << "读入所有轨迹" << endl;
	int outIndexCount = 0;
	for each (TimeSlice* timeSlice in timeSlices)//对轨迹采样点建立索引
	{
		for each (GeoPoint* point in (*timeSlice).points)
		{
			if (!routeNetwork.insertPoint(point)){
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

//输出TimeSlice
void outputTimeSlices(vector<TimeSlice*> &timeSlices){
	ofstream fout("DBScanResult_day5.txt");
	for (auto timeSlice : timeSlices){
		fout << timeSlice->time << ":" << timeSlice->clusters.size() << endl;
		for (auto cluster : timeSlice->clusters){
			for (auto object : cluster->objectIds){
				fout << object << " ";
			}
			fout << endl;
		}
	}
	fout.close();
}

//实验准备工作：读取地图匹配结果，组成各时间片的路段聚类
void edgeCluster(){
	timeSlices = vector<NewTimeSlice*>(1440);//初始化时间片集合
	int timeStamp = 0;
	for (int timeStamp = 0; timeStamp < 1440; timeStamp++){
		timeSlices.at(timeStamp) = new NewTimeSlice(timeStamp);
	}
	scanMapMatchingResultFolder(filePath, answerDirectory, timeSlices, routeNetwork);//读入地图匹配结果文件，填充时间片和路段聚类
	cout << "读入所有地图匹配结果" << endl;
	for (auto timeSlice : timeSlices)
	{
		for (auto edgeCluster : timeSlice->clusters){
			edgeCluster.second->ascertainPriorCanadidates();
		}
	}
}

//naive方法的辅助函数：判断给定的两个路段聚类是否满足扩展条件：满足返回true；否则返回false
bool couldExtendOrNot(set<int> &set1, set<int> &set2, int &intersectionCount){
	set<int> unionResult = set<int>();
	set_union(set1.begin(), set1.end(), set2.begin(), set2.end(), inserter(unionResult, unionResult.begin()));
	set<int> intersectionResult = set<int>();
	set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(), inserter(intersectionResult, intersectionResult.begin()));
	intersectionCount = static_cast<int>(intersectionResult.size());
	double similarity = (intersectionResult.size() + 0.0) / unionResult.size();
	return similarity >= DE_MINSIMILARITY;
}

//naive方法的辅助函数：在下一时间片中扩展给定的路段聚类
list<EdgeCluster*> extendDensityEdges(EdgeCluster* edgeCluster){
	list<EdgeCluster*> result = list<EdgeCluster*>();
	if (edgeCluster->time < timeSlices.size() - 1){
		for (auto tmpCluster : timeSlices.at(edgeCluster->time + 1)->clusters)
		{
			//满足最小元素个数条件和可扩展性条件
			int useless = 0;
			if (tmpCluster.second->clusterObjects.size() >= DE_MINOBJECTS&&couldExtendOrNot(edgeCluster->clusterObjects, tmpCluster.second->clusterObjects, useless)){
				tmpCluster.second->assigned = true;
				result.push_back(tmpCluster.second);
			}
		}
	}
	return result;
}

//naive方法
list<list<EdgeCluster*>> naiveMethod(){
	resultsList = list<list<EdgeCluster*>>();
	list<list<EdgeCluster*>> canadidates = list<list<EdgeCluster*>>();//候选序列集合
	for (auto timeSlice : timeSlices){
		list<list<EdgeCluster*>> newCanadidates = list<list<EdgeCluster*>>();//新的候选序列集合
		for (auto canadidate : canadidates){
			EdgeCluster* lastSnapshotCluster = canadidate.back();
			list<EdgeCluster*> assignedEdgeClusters = extendDensityEdges(lastSnapshotCluster);
			if (assignedEdgeClusters.size() == 0){
				if (canadidate.size() >= DE_DURATIVE){//满足持续性条件
					resultsList.push_back(canadidate);
				}
			}
			else{
				for (auto assignedEdgeCluster : assignedEdgeClusters){
					newCanadidates.push_back(canadidate);
					newCanadidates.back().push_back(assignedEdgeCluster);
				}
			}
		}
		for (auto edgeCluster : timeSlice->clusters){
			//当前时间片中的路段聚类未用于扩展且满足最小元素个数条件，即作为新的一个候选序列
			if ((!edgeCluster.second->assigned) && edgeCluster.second->clusterObjects.size() >= DE_MINOBJECTS){
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
list<EdgeCluster*> extendDensityEdgesWithKPruning(EdgeCluster* edgeCluster){
	list<EdgeCluster*> result = list<EdgeCluster*>();
	if (edgeCluster->time < timeSlices.size() - 1){
		for (auto tmpCluster : timeSlices.at(edgeCluster->time + 1)->clusters)
		{
			//对于带扩展时间片中的路段聚类，只有不满足k剪枝条件才可能成为序列的下一个元素
			if (tmpCluster.second->k <= tmpCluster.second->clusterObjects.size()*(1 - DE_MINSIMILARITY)){
				int intersectionCount = 0;
				//满足最小元素个数条件和可扩展性条件
				if (tmpCluster.second->clusterObjects.size() >= DE_MINOBJECTS&&couldExtendOrNot(edgeCluster->clusterObjects, tmpCluster.second->clusterObjects, intersectionCount)){
					tmpCluster.second->assigned = true;
					tmpCluster.second->k += intersectionCount;
					edgeCluster->k += intersectionCount;
					result.push_back(tmpCluster.second);
				}
				if (edgeCluster->k > edgeCluster->clusterObjects.size()*(1 - DE_MINSIMILARITY)){//对于当前序列的最后一个聚类，一旦满足k剪枝，则停止扩展
					break;
				}
			}
			else{
				continue;
			}
		}
	}
	return result;
}

//带k值剪枝的方法
list<list<EdgeCluster*>> methodWithKPruning(){
	resultsList = list<list<EdgeCluster*>>();
	list<list<EdgeCluster*>> canadidates = list<list<EdgeCluster*>>();//候选序列集合
	for (auto timeSlice : timeSlices){
		list<list<EdgeCluster*>> newCanadidates = list<list<EdgeCluster*>>();//新的候选序列集合
		for (auto canadidate : canadidates){
			EdgeCluster* lastSnapshotCluster = canadidate.back();
			lastSnapshotCluster->k = 0;
			list<EdgeCluster*> assignedEdgeClusters = extendDensityEdgesWithKPruning(lastSnapshotCluster);
			if (assignedEdgeClusters.size() == 0){
				if (canadidate.size() >= DE_DURATIVE){//满足持续性条件
					resultsList.push_back(canadidate);
				}
			}
			else{
				for (auto assignedEdgeCluster : assignedEdgeClusters){
					newCanadidates.push_back(canadidate);
					newCanadidates.back().push_back(assignedEdgeCluster);
				}
			}
		}
		for (auto edgeCluster : timeSlice->clusters){
			//当前时间片中的路段聚类未用于扩展且满足最小元素个数条件，即作为新的一个候选序列
			if ((!edgeCluster.second->assigned) && edgeCluster.second->clusterObjects.size() >= DE_MINOBJECTS){
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
list<EdgeCluster*> extendDensityEdgesWithKPruningAndMoreInfo(EdgeCluster* edgeCluster){
	list<EdgeCluster*> result = list<EdgeCluster*>();
	if (edgeCluster->time < timeSlices.size() - 1){
		for (auto edge : edgeCluster->priorCanadidates){//和带k值剪枝的方法相比，就是这句话不一样；这句话以后都是一样的
			EdgeCluster* tmpCluster = timeSlices.at(edgeCluster->time + 1)->clusters.at(edge);
			//对于带扩展时间片中的路段聚类，只有不满足k剪枝条件才可能成为序列的下一个元素
			if (tmpCluster->k <= tmpCluster->clusterObjects.size()*(1 - DE_MINSIMILARITY)){
				int intersectionCount = 0;
				//满足最小元素个数条件和可扩展性条件
				if (tmpCluster->clusterObjects.size() >= DE_MINOBJECTS&&couldExtendOrNot(edgeCluster->clusterObjects, tmpCluster->clusterObjects, intersectionCount)){
					tmpCluster->assigned = true;
					tmpCluster->k += intersectionCount;
					edgeCluster->k += intersectionCount;
					result.push_back(tmpCluster);
				}
				if (edgeCluster->k > edgeCluster->clusterObjects.size()*(1 - DE_MINSIMILARITY)){//对于当前序列的最后一个聚类，一旦满足k剪枝，则停止扩展
					break;
				}
			}
			else{
				continue;
			}
		}
	}
	return result;
}

//利用后续匹配路段信息的方法
list<list<EdgeCluster*>> methodWithKPruningAndMoreInfo(){
	resultsList = list<list<EdgeCluster*>>();
	list<list<EdgeCluster*>> canadidates = list<list<EdgeCluster*>>();//候选序列集合
	for (auto timeSlice : timeSlices){
		list<list<EdgeCluster*>> newCanadidates = list<list<EdgeCluster*>>();//新的候选序列集合
		for (auto canadidate : canadidates){
			EdgeCluster* lastSnapshotCluster = canadidate.back();
			lastSnapshotCluster->k = 0;
			list<EdgeCluster*> assignedEdgeClusters = extendDensityEdgesWithKPruningAndMoreInfo(lastSnapshotCluster);
			if (assignedEdgeClusters.size() == 0){
				if (canadidate.size() >= DE_DURATIVE){//满足持续性条件
					resultsList.push_back(canadidate);
				}
			}
			else{
				for (auto assignedEdgeCluster : assignedEdgeClusters){
					newCanadidates.push_back(canadidate);
					newCanadidates.back().push_back(assignedEdgeCluster);
				}
			}
		}
		for (auto edgeCluster : timeSlice->clusters){
			//当前时间片中的路段聚类未用于扩展且满足最小元素个数条件，即作为新的一个候选序列
			if ((!edgeCluster.second->assigned) && edgeCluster.second->clusterObjects.size() >= DE_MINOBJECTS){
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

int main(){
	//建立路网；读入地图匹配结果并构造路段聚类
	edgeCluster();//读入地图匹配结果并构造路段聚类

	//挖掘路段序列
	clock_t start, finish;
	start = clock();
	resultsList = methodWithKPruningAndMoreInfo();
	finish = clock();
	cout << "用时：" << finish - start << "毫秒" << endl;

	//评估路段序列
	cout << "共得到" << resultsList.size() << "个模式序列" << endl;
	//getDistinctEdges();
	filterInvalidEdgeSet();
	//getDistinctEdges();
	statisticDistinctEdges();

	//getTimeStatistic();
	//getAverageSpeed();

	//输出路段序列
	//outputResults("filteredResults.txt");
	//统计路段出现的频数并保存至集合distinctEdges
	//输出集合distinctEdges至Json文件
	OutputDistinctEdgesToJson(statisticDistinctEdges());
	system("pause");
	return 0;
}