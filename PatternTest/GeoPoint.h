#pragma once
#include <string>
using namespace std;

#define PI180 0.0174532925199432957694
#define LENGTH_PER_RAD 111226.29021121707545
#define INVALID_TIME -1


class GeoPoint
{
public:
	double lat;
	double lon;
	int rawTime;//原始采样时间
	int time;//注意：此处保存的已经不是原始时间戳，而是所在时间片的时间
	int id;//采样点在轨迹中的标识
	int objectId;//采样点所属轨迹的标识
	//Cluster* clusterLink;//采样点所属聚类的指针
	bool visited;//扩展属性1：用于DBSCAN算法中记录该轨迹采样点是否被访问过
	bool clustered;//扩展属性2：用于DBSCAN算法中记录该轨迹采样点是否被某个聚类包含

	GeoPoint(double lat, double lon, int time);
	GeoPoint(double lat, double lon);
	GeoPoint(int id, int objectId, double lat, double lon, int rawTime, int time);

	static double geoScale;
	static double distM(double lat1, double lon1, double lat2, double lon2);
	static double distM(GeoPoint pt1, GeoPoint pt2);
	static double distM(GeoPoint* pt1, GeoPoint* pt2);
	double distM(double lat1, double lat2);
	double distM(GeoPoint pt);
	static double distDeg(double lat1, double lon1, double lat2, double lon2);
	static double distDeg(GeoPoint pt1, GeoPoint pt2);
	static double distDeg(GeoPoint* pt1, GeoPoint* pt2);
	double distDeg(double lat1, double lat2);
	double distDeg(GeoPoint pt);
};