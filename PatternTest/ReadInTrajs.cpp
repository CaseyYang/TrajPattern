#include "ReadInTrajs.h"


//读入给定路径的轨迹文件中的一条轨迹，把每个轨迹采样点插入对应的时间片中
void readOneTrajectory(string &filePath, int trajId, vector<TimeSlice*> &timeSlices)
{
	ifstream fin(filePath);
	double lat, lon;
	int time;
	int trajSamplePointIndex = 0;
	while (fin >> time){
		if (time > 172800){
			cout << "出现超过一天的轨迹！" << "轨迹id：" << trajId << endl;
			system("pause");
		}
		fin >> lat >> lon;
		timeSlices.at((time - 86400) / 60)->points.push_back(new GeoPoint(trajSamplePointIndex, trajId, lat, lon, time));
		trajSamplePointIndex++;
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
			readOneTrajectory(folderDir + inputDirectory + "\\" + inputFileName,trajIndex,timeSlices);
			trajIndex++;
		} while (_findnext(lf, &fileInfo) == 0);
		_findclose(lf);
		return;
	}
}