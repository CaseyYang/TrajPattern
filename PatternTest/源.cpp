#include <iostream>
#include "ReadInTrajs.h"
#include "Map.h"
#include "TimeSlice.h"
#include "NewTimeSlice.h"
using namespace std;

Map routeNetwork;
string filePath = "E:\\Document\\Subjects\\Computer\Data\\新加坡轨迹数据\\";

//对比实验准备工作：读取轨迹文件、建立索引及聚类
void clusterDemo(){
	vector<TimeSlice*> timeSlices = vector<TimeSlice*>(1440);//初始化时间片集合
	int timeStamp = 0;
	for (int timeStamp = 0; timeStamp < 1440; timeStamp++){
		timeSlices.at(timeStamp) = new TimeSlice(timeStamp);
	}
	scanTrajFolder(filePath, timeSlices);//读入轨迹
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
}

void edgeCluster(){
	vector<NewTimeSlice*> timeSlices = vector<NewTimeSlice*>(1440);//初始化时间片集合
	int timeStamp = 0;
	for (int timeStamp = 0; timeStamp < 1440; timeStamp++){
		timeSlices.at(timeStamp) = new NewTimeSlice(timeStamp);
	}
	scanMapMatchingResultFolder(filePath, timeSlices, routeNetwork);//读入地图匹配结果文件
	cout << "读入所有地图匹配结果" << endl;
}

int main(){
	routeNetwork = Map(filePath, 500);//建立路网
	edgeCluster();
	return 0;
}