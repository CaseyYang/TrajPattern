#include<iostream>
#include<iterator>
#include<string>
#include"../MapLibraries/Map.h"
using namespace std;

string dataPath = "E:\\MapMatchingProject\\Data\\新加坡数据\\";
string POIfilePath = "poi.txt";
string inputDirectory = "9daysForTrajPattern\\input";
string answerDirectory = "9daysForTrajPattern\\answer";
double neighborRange = 200.0;
Map routeNetwork(dataPath, 500);
map<string,int> categories;


void generateSemanticRouteNetwork() {
	ifstream fin(POIfilePath);
	double lat, lon;
	string category;
	char separator;
	fin >> category;
	int last = 0;
	int categoryIndex = 0;
	for (int i = 0; i < category.size(); ++i) {
		if (category[i] == ',') {
			categories.insert(make_pair(category.substr(last, i-last),categoryIndex++));
			last = i + 1;
		}
	}
	while (fin >> lat >> separator >> lon >> separator >> category) {
		vector<Edge*> dest;
		routeNetwork.getNearEdges(lat, lon, neighborRange, dest);
		for each (Edge* edgePtr in dest)
		{
			++edgePtr->poiNums[categories[category]];
		}
	}
}

void main() {

}