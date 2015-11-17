#include<iostream>
#include<iterator>
#include<string>
#include"../MapLibraries/Map.h"
using namespace std;

string rootDirectory = "D:\\Document\\MDM Lab\\Data\\";
string mapDirectory = "新加坡轨迹数据\\";
string POIfilePath = "NDBC扩展\\poi.csv";
string SemanticRoadFilePath = "semanticRoad.txt";
double neighborRange = 200.0;
Map routeNetwork(rootDirectory + mapDirectory, 500);
map<string, int> categories;


void GenerateSemanticRouteNetwork() {
	//cout << "here" << endl;
	ifstream fin(rootDirectory + POIfilePath);
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
}

void OutputSemanticRouteNetwork() {
	ofstream fout(SemanticRoadFilePath);
	for each(Edge* edgePtr in routeNetwork.edges) {
		fout << edgePtr->id;
		for each(double num in edgePtr->poiNums) {
			fout << " " << num;
		}
		fout << endl;
	}
	fout.close();
}

void main() {
	GenerateSemanticRouteNetwork();
	OutputSemanticRouteNetwork();
}