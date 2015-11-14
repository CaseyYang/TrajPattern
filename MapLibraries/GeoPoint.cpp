#include <math.h>
#include "GeoPoint.h"
using namespace std;

double GeoPoint::geoScale = 6371004 * 3.141592965 / 180;

GeoPoint::GeoPoint(double lat, double lon, int rawTime) :id(-1), objectId(-1), lat(lat), lon(lon), rawTime(rawTime), time(rawTime) {
	visited = clustered = false;
	//clusterLink=NULL;
}

GeoPoint::GeoPoint(double lat, double lon) : id(-1), objectId(-1), lat(lat), lon(lon), rawTime(INVALID_TIME), time(INVALID_TIME) {
	visited = clustered = false;
	//clusterLink=NULL;
}

GeoPoint::GeoPoint(int id, int objectId, double lat, double lon, int rawTime, int time) : id(id), objectId(objectId), lat(lat), lon(lon), rawTime(rawTime), time(time) {
	visited = clustered = false;
	//clusterLink=NULL;
}

double GeoPoint::distM(double lat1, double lon1, double lat2, double lon2)
{
	double deltaLat = lat1 - lat2;
	double deltaLong = (lon2 - lon1)*cos(lat1 * PI180);
	return LENGTH_PER_RAD*sqrt(deltaLat*deltaLat + deltaLong*deltaLong);
	//return sqrt((lat1 - lat2) * (lat1 - lat2) + (lon1 - lon2) * (lon1 - lon2)) * GeoPoint::geoScale;
}

double GeoPoint::distM(GeoPoint pt1, GeoPoint pt2)
{
	return GeoPoint::distM(pt1.lat, pt1.lon, pt2.lat, pt2.lon);
}

double GeoPoint::distM(GeoPoint* pt1, GeoPoint* pt2)
{
	return GeoPoint::distM(pt1->lat, pt1->lon, pt2->lat, pt2->lon);
}

double GeoPoint::distM(double lat, double lon)
{
	double lat1 = this->lat;
	double lon1 = this->lon;
	return sqrt((lat1 - lat) * (lat1 - lat) + (lon1 - lon) * (lon1 - lon)) * GeoPoint::geoScale;
}

double GeoPoint::distM(GeoPoint pt)
{
	return GeoPoint::distM(pt.lat, pt.lon);
}

double GeoPoint::distDeg(double lat1, double lon1, double lat2, double lon2)
{
	return sqrt((lat1 - lat2) * (lat1 - lat2) + (lon1 - lon2) * (lon1 - lon2));
}

double GeoPoint::distDeg(GeoPoint pt1, GeoPoint pt2)
{
	return GeoPoint::distDeg(pt1.lat, pt1.lon, pt2.lat, pt2.lon);
}

double GeoPoint::distDeg(GeoPoint* pt1, GeoPoint* pt2)
{
	return GeoPoint::distDeg(pt1->lat, pt1->lon, pt2->lat, pt2->lon);
}

double GeoPoint::distDeg(double lat, double lon)
{
	return GeoPoint::distDeg(this->lat, this->lon, lat, lon);
}

double GeoPoint::distDeg(GeoPoint pt)
{
	return GeoPoint::distDeg(this->lat, this->lon, pt.lat, pt.lon);
}