#include "ReadInTrajs.h"


//�������·���Ĺ켣�ļ��е�һ���켣����ÿ���켣����������Ӧ��ʱ��Ƭ��
void readOneTrajectory(string &filePath, int trajId, vector<TimeSlice*> &timeSlices)
{
	ifstream fin(filePath);
	double lat, lon;
	int time;
	char useless;
	int trajSamplePointIndex = 0;
	while (fin >> time >> useless){
		if (time >= 172800){
			cout << "���ֳ���һ��Ĺ켣��" << "�켣id��" << trajId << endl;
			system("pause");
		}
		fin >> lat >> useless >> lon;
		int uniformTime = (time - 86400) / 60;//��ԭʼʱ���תΪʱ��Ƭʱ��
		timeSlices.at(uniformTime)->points.push_back(new GeoPoint(trajSamplePointIndex, trajId, lat, lon, time, uniformTime));
		trajSamplePointIndex++;
	}
	fin.close();
}

//�������·���ĵ�ͼƥ�����ļ�����ÿ��ƥ��·�β����Ӧ��ʱ��Ƭ��
void readOneMapMatchingResult(string &filePath, int trajId, vector<NewTimeSlice*> &timeSlices, Map &routeNetwork){
	ifstream fin(filePath);
	int time;
	int edgeIndex;
	double confidence;
	char useless;
	while (fin >> time >> useless){
		fin >> edgeIndex >> useless >> confidence;
		if (edgeIndex != -1){
			int uniformTime = (time - 86400) / 60;//��ԭʼʱ���תΪʱ��Ƭʱ��
			timeSlices.at(uniformTime)->add(trajId, routeNetwork.edges.at(edgeIndex));
		}
	}
	fin.close();
}

//�����ļ��������й켣�ļ�������켣��ʱ��Ƭ
void scanTrajFolder(string folderDir, vector<TimeSlice*> &timeSlices)
{
	/*�ļ�Ŀ¼�ṹΪ
	* folderDir
	* |-input
	*   |-input_000011.txt ...
	* |-output
	*   |-output_000011.txt ...
	*/
	string inputDirectory = "input";
	string completeInputFilesPath = folderDir + inputDirectory + "\\" + "*.txt";
	const char* dir = completeInputFilesPath.c_str();
	_finddata_t fileInfo;//�ļ���Ϣ
	long lf;//�ļ����
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

//�����ͼƥ�����ļ�������ʱ��Ƭ
void scanMapMatchingResultFolder(string folderDir, vector<NewTimeSlice*> &timeSlices, Map &routeNetwork){
	string inputDirectory = "splited_output";
	string completeInputFilesPath = folderDir + inputDirectory + "\\" + "*.txt";
	const char* dir = completeInputFilesPath.c_str();
	_finddata_t fileInfo;//�ļ���Ϣ
	long lf;//�ļ����
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