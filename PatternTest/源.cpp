#include<iostream>
#include"ReadInTrajs.h"
#include"Map.h"
#include"TimeSlice.h"
using namespace std;
string filePath = "D:\\Document\\MDM Lab\\Data\\�¼��¹켣����\\";

int main(){
	Map routeNetork = Map(filePath, 500);//����·��
	vector<TimeSlice*> timeSlices = vector<TimeSlice*>(1440);//��ʼ��ʱ��Ƭ����
	int timeStamp = 0;
	for (int timeStamp = 0; timeStamp < 1440; timeStamp++){
		timeSlices.at(timeStamp) = new TimeSlice(timeStamp);
	}
	scanTrajFolder(filePath, timeSlices);//����켣
	cout << "�������й켣" << endl;
	int outIndexCount = 0;
	for each (TimeSlice* timeSlice in timeSlices)//�Թ켣�����㽨������
	{
		for each (GeoPoint* point in (*timeSlice).points)
		{
			if (!routeNetork.insertPoint(point)){
				outIndexCount++;
			}
		}
	}
	cout << "�Թ켣����������ϣ�" << endl;
	cout << "����" << outIndexCount << "����������������Χ��" << endl;
	for each (TimeSlice* timeSlice in timeSlices)
	{
		timeSlice->clustering(routeNetork);
		cout << "ʱ��Ƭ" << timeSlice->time << "�õ�����" << timeSlice->clusters.size() << endl;
	}
	return 0;
}