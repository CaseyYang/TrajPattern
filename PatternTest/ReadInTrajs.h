#pragma once
#include <direct.h>
#include <io.h>
#include "TimeSlice.h"
#include "NewTimeSlice.h"
#include "Map.h"

//�������·���Ĺ켣�ļ��е�һ���켣����ÿ���켣����������Ӧ��ʱ��Ƭ��
extern void readOneTrajectory(string &filePath);
//�������·���ĵ�ͼƥ�����ļ�����ÿ��ƥ��·�β����Ӧ��ʱ��Ƭ��
void readOneMapMatchingResult(string &filePath, int trajId, vector<NewTimeSlice*> &timeSlices, Map &routeNetwork);
//�����ļ��������й켣�ļ�������켣��ʱ��Ƭ
extern void scanTrajFolder(string folderDir, vector<TimeSlice*> &timeSlices);
//�����ͼƥ�����ļ�������ʱ��Ƭ
extern void scanMapMatchingResultFolder(string folderDir, vector<NewTimeSlice*> &timeSlices, Map &routeNetwork);