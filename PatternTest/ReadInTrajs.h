#pragma once
#include <direct.h>
#include <io.h>
#include "TimeSlice.h"
#include "NewTimeSlice.h"
#include "../MapLibraries/Map.h"

//读入给定路径的轨迹文件中的一条轨迹，把每个轨迹采样点插入对应的时间片中
extern void readOneTrajectory(string &filePath, int trajId, vector<TimeSlice*> &timeSlices);
//读入给定路径的地图匹配结果文件，把每个匹配路段插入对应的时间片中
extern void readOneMapMatchingResult(string &filePath, int trajId, vector<NewTimeSlice*> &timeSlices, Map &routeNetwork);
//读入给定路径的地图匹配结果，把每个匹配路段插入对应的时间片中（对应读结果合并文件版本）
extern int readOneMapMatchingResultV2(ifstream &fin, int trajId, vector<NewTimeSlice*> &timeSlices, Map &routeNetwork);
//读入文件夹中所有轨迹文件，保存轨迹和时间片
extern void scanTrajFolder(string folderDir, string inputDirectory, vector<TimeSlice*> &timeSlices);
//读入地图匹配结果文件，保存时间片
extern void scanMapMatchingResultFolder(string folderDir, string inputDirectory, vector<NewTimeSlice*> &timeSlices, Map &routeNetwork);
//读入合并的地图匹配文件，保存时间片
extern void readMergedMapMatchingResult(string filePath, vector<NewTimeSlice*> &timeSlices, Map &routeNetwork);

//读入路段附近POI统计文件，填充每个路段的poiNums数组
extern void generateSemanticRoad(Map&routeNetwork, string filePath);
//输出路段id和poiNum数组到指定文件
extern void outputSemanticRouteNetworkToPlainText(Map&routeNetwork, string filePath);
//输出TimeSlice
extern void outputTimeSlices(vector<TimeSlice*> &timeSlices);