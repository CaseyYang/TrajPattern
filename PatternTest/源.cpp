#include <iostream>
#include "ReadInTrajs.h"
#include "Map.h"
#include "TimeSlice.h"
#include "NewTimeSlice.h"
using namespace std;

Map routeNetwork;
string filePath = "E:\\Document\\Subjects\\Computer\Data\\�¼��¹켣����\\";

//�Ա�ʵ��׼����������ȡ�켣�ļ�����������������
void clusterDemo(){
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
			if (!routeNetwork.insertPoint(point)){
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
}

void edgeCluster(){
	vector<NewTimeSlice*> timeSlices = vector<NewTimeSlice*>(1440);//��ʼ��ʱ��Ƭ����
	int timeStamp = 0;
	for (int timeStamp = 0; timeStamp < 1440; timeStamp++){
		timeSlices.at(timeStamp) = new NewTimeSlice(timeStamp);
	}
	scanMapMatchingResultFolder(filePath, timeSlices, routeNetwork);//�����ͼƥ�����ļ�
	cout << "�������е�ͼƥ����" << endl;
}

int main(){
	routeNetwork = Map(filePath, 500);//����·��
	edgeCluster();
	return 0;
}