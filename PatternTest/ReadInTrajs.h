#pragma once
#include <direct.h>
#include <io.h>
#include "TimeSlice.h"
#include "NewTimeSlice.h"
#include "../MapLibraries/Map.h"

//�������·���Ĺ켣�ļ��е�һ���켣����ÿ���켣����������Ӧ��ʱ��Ƭ��
extern void readOneTrajectory(string &filePath, int trajId, vector<TimeSlice*> &timeSlices);
//�������·���ĵ�ͼƥ�����ļ�����ÿ��ƥ��·�β����Ӧ��ʱ��Ƭ��
extern void readOneMapMatchingResult(string &filePath, int trajId, vector<NewTimeSlice*> &timeSlices, Map &routeNetwork);
//�������·���ĵ�ͼƥ��������ÿ��ƥ��·�β����Ӧ��ʱ��Ƭ�У���Ӧ������ϲ��ļ��汾��
extern int readOneMapMatchingResultV2(ifstream &fin, int trajId, vector<NewTimeSlice*> &timeSlices, Map &routeNetwork);
//�����ļ��������й켣�ļ�������켣��ʱ��Ƭ
extern void scanTrajFolder(string folderDir, string inputDirectory, vector<TimeSlice*> &timeSlices);
//�����ͼƥ�����ļ�������ʱ��Ƭ
extern void scanMapMatchingResultFolder(string folderDir, string inputDirectory, vector<NewTimeSlice*> &timeSlices, Map &routeNetwork);
//����ϲ��ĵ�ͼƥ���ļ�������ʱ��Ƭ
extern void readMergedMapMatchingResult(string filePath, vector<NewTimeSlice*> &timeSlices, Map &routeNetwork);

//����·�θ���POIͳ���ļ������ÿ��·�ε�poiNums����
extern void generateSemanticRoad(Map&routeNetwork, string filePath);
//���·��id��poiNum���鵽ָ���ļ�
extern void outputSemanticRouteNetworkToPlainText(Map&routeNetwork, string filePath);
//���TimeSlice
extern void outputTimeSlices(vector<TimeSlice*> &timeSlices);