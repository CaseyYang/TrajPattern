#pragma once
#include <direct.h>
#include <io.h>
#include "TimeSlice.h"
#include "../../MapLibraries/Map.h"

//�������·���Ĺ켣�ļ��е�һ���켣
extern void readOneTrajectory(string &filePath);
//�����ļ��������й켣�ļ�������켣�����ɶ�Ӧ������ļ���
extern void scanTrajFolder(string folderDir, vector<TimeSlice*> &timeSlices);