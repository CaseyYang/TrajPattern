#include <sstream>
#include "ReadInTrajs.h"

int day = 3;

string ToString(int i){
	stringstream ss;
	ss << i;
	return ss.str();
}

//读入给定路径的轨迹文件中的一条轨迹，把每个轨迹采样点插入对应的时间片中
void readOneTrajectory(string &filePath, int trajId, vector<TimeSlice*> &timeSlices)
{
	ifstream fin(filePath);
	double lat, lon;
	int time, lastUniformTime = -1;//由于一些采样点的采样间隔小于1分钟，使用lastUniformTime用于防止同一个时间片中插入多个来自同一轨迹的采样点
	char useless;
	int trajSamplePointIndex = 0;
	while (fin >> time >> useless){
		if (time >= 172800){
			cout << "出现超过一天的轨迹！" << "轨迹id：" << trajId << endl;
			system("pause");
		}
		fin >> lat >> useless >> lon;
		int uniformTime = (time - 86400) / 60;//把原始时间戳转为时间片时间
		if (uniformTime > lastUniformTime){
			timeSlices.at(uniformTime)->points.push_back(new GeoPoint(trajSamplePointIndex, trajId, lat, lon, time, uniformTime));
			trajSamplePointIndex++;
			lastUniformTime = uniformTime;
		}
	}
	fin.close();
}

//读入给定路径的地图匹配结果文件，把每个匹配路段插入对应的时间片中
void readOneMapMatchingResult(string &filePath, int trajId, vector<NewTimeSlice*> &timeSlices, Map &routeNetwork){
	ifstream fin(filePath);
	int time, lastUniformTime = -1;//uniformTime作用同readOneTrajectory函数中lastUniformTime
	int edgeIndex;
	double confidence;
	char useless;
	EdgeCluster* lastEdgeCluster = NULL;//记录上一个路段聚类，因为要用当前的匹配路段来更新上一路段聚类的nextEdgeCounts集合
	while (fin >> time >> useless){
		fin >> edgeIndex >> useless >> confidence;
		int uniformTime = time / 60;//把原始时间戳转为时间片时间
		if (edgeIndex != -1 && uniformTime > lastUniformTime){
			Edge* matchedEdge = routeNetwork.edges.at(edgeIndex);
			timeSlices.at(uniformTime)->add(trajId, matchedEdge);
			if (lastEdgeCluster != NULL&&uniformTime == lastUniformTime + 1){//上一个路段聚类不为空
				lastEdgeCluster->refreshNextEdgeCounts(matchedEdge);//更新上一路段聚类的nextEdgeCounts集合
			}
			lastEdgeCluster = timeSlices.at(uniformTime)->clusters.at(matchedEdge);//更新lastEdgeCluster指向当前路段聚类
			lastUniformTime = uniformTime;
		}
	}
	fin.close();
}

//读入文件夹中所有轨迹文件，保存轨迹和时间片
void scanTrajFolder(string folderDir,string inputDirectory, vector<TimeSlice*> &timeSlices)
{
	/*文件目录结构为
	* folderDir
	* |-input
	*   |-input_000011.txt ...
	* |-output
	*   |-output_000011.txt ...
	*/
	string completeInputFilesPath = folderDir + inputDirectory + "\\" + "*.txt";
	const char* dir = completeInputFilesPath.c_str();
	_finddata_t fileInfo;//文件信息
	long lf;//文件句柄
	if ((lf = _findfirst(dir, &fileInfo)) == -1l) {
		cout << "文件夹" << completeInputFilesPath << "下未找到指定文件！" << endl;
		return;
	}
	else {
		int trajIndex = 0;
		do {
			string inputFileName = fileInfo.name;
			readOneTrajectory(folderDir + inputDirectory + "\\" + inputFileName, trajIndex, timeSlices);
			trajIndex++;
		} while (_findnext(lf, &fileInfo) == 0);
		_findclose(lf);
		cout << "所有轨迹文件读入完毕！" << endl;
		return;
	}
}

//读入地图匹配结果文件，保存时间片
void scanMapMatchingResultFolder(string folderDir, string inputDirectory, vector<NewTimeSlice*> &timeSlices, Map &routeNetwork){
	string completeInputFilesPath = folderDir + inputDirectory + "\\" + "*.txt";
	const char* dir = completeInputFilesPath.c_str();
	_finddata_t fileInfo;//文件信息
	long lf;//文件句柄
	if ((lf = _findfirst(dir, &fileInfo)) == -1l) {
		return;
	}
	else {
		int trajIndex = 0;
		do {
			string inputFileName = fileInfo.name;
			readOneMapMatchingResult(folderDir + inputDirectory + "\\" + inputFileName, trajIndex, timeSlices, routeNetwork);
			trajIndex++;
		} while (_findnext(lf, &fileInfo) == 0);
		_findclose(lf);
		return;
	}
}