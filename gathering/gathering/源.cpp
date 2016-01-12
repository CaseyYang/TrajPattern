#include<iostream>
#include<time.h>
#include<sstream>
#include"ReadInTrajs.h"
#include "../../MapLibraries/Map.h"
#include"TimeSlice.h"
using namespace std;
//string filePath = "D:\\MapMatchingProject\\Data\\新加坡数据\\";
string filePath = "I:\\YangKaixi\\MapMatchingProject\\MapMatchingProject\\Data\\";


//const int maxn = 1000; //!!!!!!!!!
const double sigma = 200; //300
const int Kc = 10; //7
const int Mc = 10; //5
const int Kp = 5; //3
const int Mp = 5; //3
const int TrajCount = 200000; //!!!!!!!

Map routeNetork = Map(filePath + "新加坡路网\\", 500);//建立路网
vector<TimeSlice*> timeSlices;
list<list<Cluster*>> curCrow, cloCrow, Temp, Ans;
list<Cluster*> curT, R, C;
int num[TrajCount];
list<Cluster*> Tem;



//Hausdorff距离
list<Cluster*> rangeSearch(Cluster* cluster, list<Cluster*> &clusters, double dist, int mc) {
	list<Cluster*> result = list<Cluster*>();
	for (auto cluster2 : clusters) {
		if (cluster2->clusterObjects.size() >= mc) {
			map<GeoPoint*, double> dists = map<GeoPoint*, double>();
			double tmp1Max = -1;
			double tmp2Max = -1;
			for (auto point2 : cluster2->clusterObjects) {
				double tmp1Min = 10000000;
				for (auto point : cluster->clusterObjects) {
					double tmp1Dist = GeoPoint::distM(point, point2);
					if (tmp1Dist < tmp1Min) {
						tmp1Min = tmp1Dist;
					}

					if (dists.find(point) == dists.end()) {
						dists.insert(make_pair(point, tmp1Dist));
					}
					else {
						if (dists.at(point)>tmp1Dist) {
							dists.at(point) = tmp1Dist;
						}
					}
				}
				if (tmp1Min > tmp1Max) {
					tmp1Max = tmp1Min;
				}
			}
			for (auto ddd : dists) {
				if (ddd.second > tmp2Max) {
					tmp2Max = ddd.second;
				}
			}
			tmp1Max = tmp2Max > tmp1Max ? tmp2Max : tmp1Max;
			if (tmp1Max <= dist) {
				result.push_back(cluster2);
			}
			dists.clear();
		}
	}
	return result;
}



void DiscoveringClosedCrowds() {
	int sz = static_cast<int>(timeSlices.size()), tot = 0;
	for (list<Cluster*>::iterator it = timeSlices[0]->clusters.begin(); it != timeSlices[0]->clusters.end(); it++) {
		if ((*it)->clusterObjects.size() >= Mc) {
			Tem.clear(); Tem.push_back(*it);//什么意思？
			curCrow.push_back(Tem);
		}
	}
	for (int i = 1; i < sz; i++) {
		Temp.clear();
		for (list<list<Cluster*>>::iterator it = curCrow.begin(); it != curCrow.end();) {
			curT = rangeSearch((*it).back(), timeSlices[i]->clusters, sigma, Mc);
			if (curT.size() == 0) {
				if ((*it).size() >= Kc)
					cloCrow.push_back(*it);
			}
			else {
				for (list<Cluster*>::iterator ip = curT.begin(); ip != curT.end(); ip++) {
					(*ip)->flag = true;
					if ((*ip)->clusterObjects.size() >= Mc) {
						Tem.clear(); Tem = (*it);
						Tem.push_back(*ip);
						Temp.push_back(Tem);
					}
				}
			}
			it = curCrow.erase(it);
		}
		for (list<list<Cluster*>>::iterator it = Temp.begin(); it != Temp.end(); it++)
			curCrow.push_back(*it);
		for (list<Cluster*>::iterator it = timeSlices[i]->clusters.begin(); it != timeSlices[i]->clusters.end(); it++) {
			if (!(*it)->flag) {
				Tem.clear(); Tem.push_back(*it);
				curCrow.push_back(Tem);
			}
		}
	}
	for (list<list<Cluster*>>::iterator it = curCrow.begin(); it != curCrow.end(); it++) {
		if ((*it).size() >= Kc) cloCrow.push_back((*it));
	}
}


bool Test(list<Cluster*> &Cr, int Kp, int Mp) {
	set<int> Obj; Obj.clear(); C.clear();
	for (list<Cluster*>::iterator it = Cr.begin(); it != Cr.end(); it++) {
		for (set<GeoPoint*>::iterator ip = (*it)->clusterObjects.begin(); ip != (*it)->clusterObjects.end(); ip++) {
			int t = (*ip)->objectId;
			if (Obj.count(t) == 0) num[t] = 0, Obj.insert(t);
			num[t]++;
		}
	}
	bool flag = true;
	for (list<Cluster*>::iterator it = Cr.begin(); it != Cr.end(); it++) {
		int cnt = 0;
		for (set<GeoPoint*>::iterator ip = (*it)->clusterObjects.begin(); ip != (*it)->clusterObjects.end(); ip++) {
			int t = (*ip)->objectId;
			if (num[t] >= Kp) cnt++;
		}
		if (cnt < Mp) {
			flag = false; C.push_back((*it));
		}
	}
	return flag;
}

list<list<Cluster*>> Divide(list<Cluster*> &Cr, list<Cluster*> &C) {
	list<list<Cluster*>> R; R.clear();
	list<Cluster*> Tem; Tem.clear();
	list<Cluster*>::iterator it = Cr.begin(), ip = C.begin();
	while (it != Cr.end()) {
		if (ip != C.end()) {
			if ((*it) == (*ip)) {
				if (!Tem.empty())  R.push_back(Tem);
				it++; ip++; Tem.clear();
			}
			else {
				Tem.push_back(*it);
				it++;
			}
		}
		else {
			Tem.push_back(*it);
			it++;
		}
	}
	/*if (R.size() == 0) {
		cout<<"------Cr"<<endl;
		for (it = Cr.begin(); it != Cr.end(); it++)
			cout << (*it)->time << " ";
		cout << endl;
		cout << "------C" << endl;
		for (it = C.begin(); it != C.end(); it++)
			cout << (*it)->time << " ";
		cout << endl;
		cout << "------C" << endl;
		for (it = Tem.begin(); it != Tem.end(); it++)
			cout << (*it)->time << " ";
		cout << endl;
	}*/
	if (!Tem.empty()) {
		R.push_back(Tem);
	}
	return R;
}

list<list<Cluster*>> TestAndDivide(list<Cluster*> &Cr, int Kc, int Kp, int Mp) {
	list<list<Cluster*>> R; R.clear();
	if (Test(Cr, Kp, Mp)) {
		R.push_back(Cr);
	}
	else {
		list<list<Cluster*>> Scr = Divide(Cr, C);
		for (list<list<Cluster*>>::iterator it = Scr.begin(); it != Scr.end(); it++) {
			if ((*it).size() >= Kc) {
				Temp = TestAndDivide((*it), Kc, Kp, Mp);
				for (list<list<Cluster*>>::iterator ip = Temp.begin(); ip != Temp.end(); ip++) {
					R.push_back(*ip);
				}
			}
		}
	}
	return R;
}

//对结果进行统计处理
//输出离结果序列首尾两个聚类附近的路段
void resultProcess() {
	int index = 0;
	for (auto ans : Ans) {
		Cluster* firstCluster = ans.front();
		Cluster* lastCluster = ans.back();
		set<Edge*> firstClusterEdges = set<Edge*>(), lastClusterEdges = set<Edge*>();
		double shortestDist = 0;
		for (auto obj : firstCluster->clusterObjects) {
			Edge* edge = routeNetork.getNearestEdge(obj->lat, obj->lon, shortestDist);
			if (edge != NULL) firstClusterEdges.insert(edge);
		}
		for (auto obj : lastCluster->clusterObjects) {
			Edge* edge = routeNetork.getNearestEdge(obj->lat, obj->lon, shortestDist);
			if (edge != NULL) lastClusterEdges.insert(edge);
		}
		stringstream ss;
		ss << "edges_" << ++index<<".txt";
		ofstream fout(ss.str());
		for (auto edge : firstClusterEdges) {
			fout << edge->id << endl;
		}
		fout << -1 << endl;
		for (auto edge : lastClusterEdges) {
			fout << edge->id << endl;
		}
		fout << -1 << endl;
		fout.close();
	}
}

//void scanOneTrajectory(string &filePath, int trajId, int rawtime)
//{
//	ifstream fin(filePath);
//	double lat, lon;
//	int time, idt;
//	char useless;
//	static int trajSamplePointIndex = 0;
//	int lastUniformTime = -1;
//	while (fin >> time >> useless) {
//		if (time >= 172800) {
//			cout << "出现超过一天的轨迹！" << "轨迹id：" << trajId << endl;
//			system("pause");
//		}
//		fin >> idt >> useless >> lon;
//
//	}
//	fin.close();
//}


int main() {
	timeSlices = vector<TimeSlice*>(1440);//初始化时间片集合
	int timeStamp = 0;
	for (int timeStamp = 0; timeStamp < 1440; timeStamp++) {
		timeSlices.at(timeStamp) = new TimeSlice(timeStamp);
	}
	clock_t start = clock();
	scanTrajFolder(filePath, timeSlices);//读入轨迹，注意这里要进入函数修改轨迹文件目录！！！
	clock_t end = clock();
	cout << "读入所有轨迹！用时" << end - start << "毫秒" << endl;
	int outIndexCount = 0;
	for each (TimeSlice* timeSlice in timeSlices)//对轨迹采样点建立索引
	{
		for each (GeoPoint* point in (*timeSlice).points)
		{
			if (!routeNetork.insertPoint(point)) {
				outIndexCount++;
			}
		}
	}
	
	cout << "对轨迹建立索引完毕！" << endl;
	cout << "共有" << outIndexCount << "个采样点在索引范围外" << endl;
	start = end;
	for each (TimeSlice* timeSlice in timeSlices)
	{
		timeSlice->clustering(routeNetork);
		if (timeSlice->time == 100 || timeSlice->time == 500 || timeSlice->time == 1000) {
			cout << "完成至" << timeSlice->time << endl;
		}
	}
	end = clock();
	cout << "聚类结束！用时" << end - start << "毫秒" << endl;
	start = end;
	DiscoveringClosedCrowds();

	//ofstream fout("debug.txt");
	//fout << "cloCrow number: " << cloCrow.size() << endl;
	double r = 0, cntw = 0;
	for (list<list<Cluster*>>::iterator ip = cloCrow.begin(); ip != cloCrow.end(); ip++) {
		r += (*ip).size(); cntw++;
	}
	//fout << r / cntw << endl;
	//int cnt = 0;
	for (list<list<Cluster*>>::iterator it = cloCrow.begin(); it != cloCrow.end(); it++) {
		//Ans = TestAndDivide(*it, Kc, Kp, Mp);
		int ansSize = static_cast<int>(Ans.size());
		auto curAns = TestAndDivide(*it, Kc, Kp, Mp);
		Ans.merge(curAns);
		//for (list<list<Cluster*>>::iterator ip = Ans.begin(); ip != Ans.end(); ip++) {
		//	//fout << (*ip).size() << endl;
		//	cnt++;
		//}
	}
	end = clock();
	cout << "用时" << end - start << "毫秒" << endl;
	cout << "结果序列数量：" << Ans.size() << endl;
	//fout << "Total Number: " << cnt << endl;
	resultProcess();
	system("pause");
	return 0;
}