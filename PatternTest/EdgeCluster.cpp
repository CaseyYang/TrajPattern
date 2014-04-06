#include "EdgeCluster.h"
using namespace std;

EdgeCluster::EdgeCluster(Edge* edge, int time) :clusterCoreEdge(edge), time(time){
	clusterObjects = set<int>();
}

bool EdgeCluster::add(int objectId){
	if (clusterObjects.find(objectId) == clusterObjects.end()){
		clusterObjects.insert(objectId);
		return true;
	}
	else{
		return false;
	}
}