#pragma once
#include <direct.h>
#include <io.h>
#include "TimeSlice.h"
#include "../../MapLibraries/Map.h"

//读入给定路径的轨迹文件中的一条轨迹
extern void readOneTrajectory(string &filePath);
//读入文件夹中所有轨迹文件，保存轨迹并生成对应的输出文件名
extern void scanTrajFolder(string folderDir, vector<TimeSlice*> &timeSlices);