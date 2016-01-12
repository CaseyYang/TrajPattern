#include "ReadInTrajs.h"


//�������·���Ĺ켣�ļ��е�һ���켣����ÿ���켣����������Ӧ��ʱ��Ƭ��
void readOneTrajectory(string &filePath, int trajId, vector<TimeSlice*> &timeSlices)
{
	ifstream fin(filePath);
	double lat, lon;
	int time;
	char useless;
	static int trajSamplePointIndex = 0;
	int lastUniformTime = -1;
	while (fin >> time >> useless){
		if (time >= 172800){
			cout << "���ֳ���һ��Ĺ켣��" << "�켣id��" << trajId << endl;
			system("pause");
		}
		fin >> lat >> useless >> lon;
		//int uniformTime = (time-86400) / 60;//��ԭʼʱ���תΪʱ��Ƭʱ��
		int uniformTime = time / 60;
		if (lastUniformTime < uniformTime) {
			timeSlices.at(uniformTime)->points.push_back(new GeoPoint(trajSamplePointIndex, trajId, lat, lon, time, uniformTime));
			trajSamplePointIndex++;
		}
			lastUniformTime = uniformTime;
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
	string inputDirectory = "day1\\day1_unsplited_input";
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