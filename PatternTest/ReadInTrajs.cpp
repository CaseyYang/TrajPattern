#include "ReadInTrajs.h"


//读入给定路径的轨迹文件中的一条轨迹，把每个轨迹采样点插入对应的时间片中
void readOneTrajectory(string &filePath, int trajId, vector<TimeSlice*> &timeSlices)
{
	ifstream fin(filePath);
	double lat, lon;
	int time;
	char useless;
	int trajSamplePointIndex = 0;
	while (fin >> time >> useless){
		if (time >= 172800){
			cout << "出现超过一天的轨迹！" << "轨迹id：" << trajId << endl;
			system("pause");
		}
		fin >> lat >> useless >> lon;
		int uniformTime = (time - 86400) / 60;//把原始时间戳转为时间片时间
		timeSlices.at(uniformTime)->points.push_back(new GeoPoint(trajSamplePointIndex, trajId, lat, lon, time, uniformTime));
		trajSamplePointIndex++;
	}
	fin.close();
}

//读入给定路径的地图匹配结果文件，把每个匹配路段插入对应的时间片中
void readOneMapMatchingResult(string &filePath, int trajId, vector<NewTimeSlice*> &timeSlices, Map &routeNetwork){
	ifstream fin(filePath);
	int time;
	int edgeIndex;
	double confidence;
	char useless;
	while (fin >> time >> useless){
		fin >> edgeIndex >> useless >> confidence;
		if (edgeIndex != -1){
			int uniformTime = (time - 86400) / 60;//把原始时间戳转为时间片时间
			timeSlices.at(uniformTime)->add(trajId, routeNetwork.edges.at(edgeIndex));
		}
	}
	fin.close();
}

//读入文件夹中所有轨迹文件，保存轨迹和时间片
void scanTrajFolder(string folderDir, vector<TimeSlice*> &timeSlices)
{
	/*文件目录结构为
	* folderDir
	* |-input
	*   |-input_000011.txt ...
	* |-output
	*   |-output_000011.txt ...
	*/
	string inputDirectory = "input";
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
			readOneTrajectory(folderDir + inputDirectory + "\\" + inputFileName, trajIndex, timeSlices);
			trajIndex++;
		} while (_findnext(lf, &fileInfo) == 0);
		_findclose(lf);
		return;
	}
}

//读入地图匹配结果文件，保存时间片
void scanMapMatchingResultFolder(string folderDir, vector<NewTimeSlice*> &timeSlices, Map &routeNetwork){
	string inputDirectory = "splited_output";
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