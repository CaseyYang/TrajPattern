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


void GenerateSemanticRouteNetwork() {
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

void main() {
	GenerateSemanticRouteNetwork();
}