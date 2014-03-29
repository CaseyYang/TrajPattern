#include<iostream>
#include"ReadInTrajs.h"
#include"Map.h"
#include"TimeSlice.h"
using namespace std;
string filePath = "";

int main(){
	Map routeNetork = Map(filePath, 500);//����·��
	vector<TimeSlice*> timeSlices = vector<TimeSlice*>(1440);//��ʼ��ʱ��Ƭ����
	//cout << timeSlices.size() << endl;
	scanTrajFolder(filePath, timeSlices);//����켣
	for each (TimeSlice* timeSlice in timeSlices)//�Թ켣�����㽨������
	{
		for each (TrajSamplePoint* point in (*timeSlice).points)
		{
			if (!routeNetork.insertPoint(point)){
				cout << "�в����㳬��������Χ��������id��" << point->id << "���켣id" << point->objectId << endl;
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