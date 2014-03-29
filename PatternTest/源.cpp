#include<iostream>
#include"ReadInTrajs.h"
#include"Map.h"
#include"TimeSlice.h"
using namespace std;
string filePath = "";

int main(){
	Map routeNetork = Map(filePath, 500);//建立路网
	vector<TimeSlice*> timeSlices = vector<TimeSlice*>(1440);//初始化时间片集合
	//cout << timeSlices.size() << endl;
	scanTrajFolder(filePath, timeSlices);//读入轨迹
	for each (TimeSlice* timeSlice in timeSlices)//对轨迹采样点建立索引
	{
		for each (TrajSamplePoint* point in (*timeSlice).points)
		{
			if (!routeNetork.insertPoint(point)){
				cout << "有采样点超出索引范围！采样点id：" << point->id << "，轨迹id" << point->objectId << endl;
				system("pause");
			}
		}
	}
	for each (TimeSlice* timeSlice in timeSlices)
	{
		timeSlice.clustering(routeNetork);
	}
	return 0;
}