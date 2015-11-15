#include <iostream>
#include <iterator>
#include "../MapLibraries/Map.h"
using namespace std;

string filePath = "E:\\MapMatchingProject\\Data\\新加坡数据\\";
string POIfilePath = "poi.txt";
string inputDirectory = "9daysForTrajPattern\\input";
string answerDirectory = "9daysForTrajPattern\\answer";
Map routeNetwork(filePath, 500);

void main() {
	ifstream fin(POIfilePath);
}