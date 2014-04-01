#include<iostream>
#include"ReadInTrajs.h"
#include"Map.h"
#include"TimeSlice.h"
using namespace std;
string filePath = "D:\\Document\\MDM Lab\\Data\\新加坡轨迹数据\\";

int main(){
	Map routeNetork = Map(filePath, 500);//建立路网
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
			if (!routeNetork.insertPoint(point)){
				outIndexCount++;
			}
		}
	}
	cout << "对轨迹建立索引完毕！" << endl;
	cout << "共有" << outIndexCount << "个采样点在索引范围外" << endl;
	for each (TimeSlice* timeSlice in timeSlices)
	{
		timeSlice->clustering(routeNetork);
		cout << "时间片" << timeSlice->time << "得到聚类" << timeSlice->clusters.size() << endl;
	}
	return 0;
}