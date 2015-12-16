#include<iostream>
#include<iterator>
#include<string>
#include"../MapLibraries/Map.h"
using namespace std;

string rootDirectory = "D:\\Document\\MDM Lab\\Data\\";
string mapDirectory = "新加坡轨迹数据\\";
string poiFilePath = "NDBC扩展\\poi.csv";
string semanticRoadFilePath = "semanticRoad.txt";
double neighborRange = 200.0;
Map routeNetwork(rootDirectory + mapDirectory, 500);
map<string, int> categories;

//读入POI数据，依此填充路段的poiNums数组
//对于每个POI点，寻找neighborRange范围内的路段，在路段的poiNums数组中对应POI种类计数加1
void generateSemanticRouteNetwork() {
	ifstream fin(rootDirectory + poiFilePath);
	double lat, lon;
	string category;
	char separator;
	fin >> category;
	int last = 0;
	int categoryIndex = 0;
	for (int i = 0; i < category.size(); ++i) {
		if (category[i] == ',') {
			categories.insert(make_pair(category.substr(last, i - last), categoryIndex++));
			last = i + 1;
		}
	}
	categories.insert(make_pair(category.substr(last, category.size() - last), categoryIndex++));
	while (fin >> lat >> separator >> lon >> separator >> category) {
		vector<Edge*> dest;
		routeNetwork.getNearEdges(lat, lon, neighborRange, dest);
		for each (Edge* edgePtr in dest)
		{
			if (edgePtr->poiNums.size() == 0) {
				edgePtr->poiNums = vector<double>(categories.size());
			}
			++edgePtr->poiNums[categories[category]];
		}
	}
	fin.close();
}

//对每个路段的poiNums数组进行归一化处理，每个POI种类数量除以该路段附近总的POI数量
void poiNumsNormalize() {
	for each (Edge* edge in routeNetwork.edges)
	{
		if (edge == NULL) continue;
		int count = 0;
		for each(double num in edge->poiNums) {
			count += static_cast<int>(num);
		}
		if (count == 0) continue;
		for (int i = 0; i < edge->poiNums.size(); ++i) {
			edge->poiNums[i] = edge->poiNums[i] / count;
		}
	}
}


//输出路段的poiNums数组至指定文件
//文件格式为：
//第一行：		POI种类名称（逗号分隔）
//第二行及以后：	路段Id,第一种POI的归一化数量,第二种POI的归一化数量,……最后一种POI的归一化数量,路段所属语义聚类Id（即globalSemanticType）
void outputSemanticRouteNetwork(string filePath) {
	ofstream fout(filePath);
	bool first = true;
	for each (pair<string, int> category in categories)
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
		fout << "," << edgePtr->globalSemanticType<<endl;
	}
	fout.close();
}

void main() {
	generateSemanticRouteNetwork();
	poiNumsNormalize();
	//TODO: Clustering Function
	outputSemanticRouteNetwork(semanticRoadFilePath);
}