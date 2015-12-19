#include <sstream>
#include "ReadInTrajs.h"

int day = 3;

string ToString(int i) {
	stringstream ss;
	ss << i;
	return ss.str();
}

//�������·���Ĺ켣�ļ��е�һ���켣����ÿ���켣����������Ӧ��ʱ��Ƭ��
void readOneTrajectory(string &filePath, int trajId, vector<TimeSlice*> &timeSlices)
{
	ifstream fin(filePath);
	double lat, lon;
	int time, lastUniformTime = -1;//����һЩ������Ĳ������С��1���ӣ�ʹ��lastUniformTime���ڷ�ֹͬһ��ʱ��Ƭ�в���������ͬһ�켣�Ĳ�����
	char useless;
	int trajSamplePointIndex = 0;
	while (fin >> time >> useless) {
		if (time >= 172800) {
			cout << "���ֳ���һ��Ĺ켣��" << "�켣id��" << trajId << endl;
			system("pause");
		}
		fin >> lat >> useless >> lon;
		int uniformTime = (time - 86400) / 60;//��ԭʼʱ���תΪʱ��Ƭʱ��
		if (uniformTime > lastUniformTime) {
			timeSlices.at(uniformTime)->points.push_back(new GeoPoint(trajSamplePointIndex, trajId, lat, lon, time, uniformTime));
			trajSamplePointIndex++;
			lastUniformTime = uniformTime;
		}
	}
	fin.close();
}

//�������·���ĵ�ͼƥ�����ļ�����ÿ��ƥ��·�β����Ӧ��ʱ��Ƭ��
void readOneMapMatchingResult(string &filePath, int trajId, vector<NewTimeSlice*> &timeSlices, Map &routeNetwork) {
	ifstream fin(filePath);
	int time, lastUniformTime = -1;//uniformTime����ͬreadOneTrajectory������lastUniformTime
	int edgeIndex;
	double confidence;
	char useless;
	EdgeCluster* lastEdgeCluster = NULL;//��¼��һ��·�ξ��࣬��ΪҪ�õ�ǰ��ƥ��·����������һ·�ξ����nextEdgeCounts����
	while (fin >> time >> useless) {
		fin >> edgeIndex >> useless >> confidence;
		int uniformTime = time / 60;//��ԭʼʱ���תΪʱ��Ƭʱ��
		if (edgeIndex != -1 && uniformTime > lastUniformTime) {
			Edge* matchedEdge = routeNetwork.edges.at(edgeIndex);
			timeSlices.at(uniformTime)->add(trajId, matchedEdge);
			if (lastEdgeCluster != NULL&&uniformTime == lastUniformTime + 1) {//��һ��·�ξ��಻Ϊ��
				lastEdgeCluster->refreshNextEdgeCounts(matchedEdge);//������һ·�ξ����nextEdgeCounts����
			}
			lastEdgeCluster = timeSlices.at(uniformTime)->clusters.at(matchedEdge);//����lastEdgeClusterָ��ǰ·�ξ���
			lastUniformTime = uniformTime;
		}
	}
	fin.close();
}

//�����ļ��������й켣�ļ�������켣��ʱ��Ƭ
void scanTrajFolder(string folderDir, string inputDirectory, vector<TimeSlice*> &timeSlices)
{
	/*�ļ�Ŀ¼�ṹΪ
	* folderDir
	* |-input
	*   |-input_000011.txt ...
	* |-output
	*   |-output_000011.txt ...
	*/
	string completeInputFilesPath = folderDir + inputDirectory + "\\" + "*.txt";
	const char* dir = completeInputFilesPath.c_str();
	_finddata_t fileInfo;//�ļ���Ϣ
	intptr_t lf;//�ļ����
	if ((lf = _findfirst(dir, &fileInfo)) == -1l) {
		cout << "�ļ���" << completeInputFilesPath << "��δ�ҵ�ָ���ļ���" << endl;
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
		cout << "���й켣�ļ�������ϣ�" << endl;
		return;
	}
}

//�����ͼƥ�����ļ�������ʱ��Ƭ
void scanMapMatchingResultFolder(string folderDir, string inputDirectory, vector<NewTimeSlice*> &timeSlices, Map &routeNetwork) {
	string completeInputFilesPath = folderDir + inputDirectory + "\\" + "*.txt";
	const char* dir = completeInputFilesPath.c_str();
	_finddata_t fileInfo;//�ļ���Ϣ
	intptr_t lf;//�ļ����
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

//����·�θ���POIͳ���ļ������ÿ��·�ε�poiNums����
void generateSemanticRoad(Map&routeNetwork, string filePath) {
	ifstream fin(filePath);
	string rawStr;
	fin >> rawStr;
	int index = 0;
	int last = 0;
	for (int i = 0; i < rawStr.size(); ++i) {
		if (rawStr[i] == ',') {
			string category = rawStr.substr(last, i - last);
			Edge::poiCategories.insert(make_pair(category, index++));
			last = i + 1;
		}
	}
	string category = rawStr.substr(last, rawStr.size() - last);
	Edge::poiCategories.insert(make_pair(category, index++));
	vector<Edge*>::iterator edgeIter = routeNetwork.edges.begin();
	while (fin >> rawStr) {
		int last = 0;
		int separatorNum = 0;
		for (int i = 0; i < rawStr.size(); ++i) {
			if (rawStr[i] == ',') {
				++separatorNum;
				string curStr = rawStr.substr(last, i - last);
				int curNum = atoi(curStr.c_str());
				last = i + 1;
				if (separatorNum == 1) {
					if ((*edgeIter)->id != curNum) {
						cout << "��ǰö�ٵ�·��Id�ʹ��ļ��ж�ȡ��·��Id������" << endl;
						cout << "��ǰö�ٵ�·��Id" << (*edgeIter)->id << " ���ļ��ж�ȡ��·��Id" << curNum << endl;
						system("pause");
					}
				}
				else {
					if ((*edgeIter)->poiNums.size() == 0) (*edgeIter)->poiNums = vector<double>(Edge::poiCategories.size());
					(*edgeIter)->poiNums[separatorNum - 2] = curNum;
				}
			}
		}
		//���һ�����ŷָ�����·�ε��������Id
		if (separatorNum > 1) {
			string curStr = rawStr.substr(last, rawStr.size() - last);
			int curNum = atoi(curStr.c_str());
			(*edgeIter)->globalSemanticType = curNum;
		}
		++edgeIter;
		while (*edgeIter == NULL) ++edgeIter;
	}
	fin.close();
}

//���·�ε�poiNums������ָ���ļ�
//�ļ���ʽΪ��
//��һ�У�		POI�������ƣ����ŷָ���
//�ڶ��м��Ժ�	·��Id,��һ��POI�Ĺ�һ������,�ڶ���POI�Ĺ�һ������,�������һ��POI�Ĺ�һ������,·�������������Id����globalSemanticType��
void outputSemanticRouteNetworkToPlainText(Map&routeNetwork,string filePath) {
	ofstream fout(filePath);
	bool first = true;
	for each (pair<string, int> category in Edge::poiCategories)
	{
		if (first) {
			first = false;
		}
		else {
			fout << ",";
		}
		fout << category.first;
	}
	fout << endl;
	for each(Edge* edgePtr in routeNetwork.edges) {
		if (edgePtr == NULL) continue;
		fout << edgePtr->id;
		for each(double num in edgePtr->poiNums) {
			fout << "," << num;
		}
		fout << "," << edgePtr->globalSemanticType << endl;
	}
	fout.close();
}

//���TimeSlice
void outputTimeSlices(vector<TimeSlice*> &timeSlices) {
	ofstream fout("DBScanResult_day5.txt");
	for (auto timeSlice : timeSlices) {
		fout << timeSlice->time << ":" << timeSlice->clusters.size() << endl;
		for (auto cluster : timeSlice->clusters) {
			for (auto object : cluster->objectIds) {
				fout << object << " ";
			}
			fout << endl;
		}
	}
	fout.close();
}