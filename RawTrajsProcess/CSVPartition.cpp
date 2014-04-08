#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>

using namespace std;

struct GeoPoint 
{
	double lat;
	double lon;
	int time;

	GeoPoint(int time, double lat, double lon)
	{
		this->time = time;
		this->lat = lat;
		this->lon = lon;
	}
};

typedef list<GeoPoint*> Traj;

void split(const string& src, const char& separator, vector<string>& dest)
{
	int index = 0, start = 0;
	while (index != src.size())
	{
		if (src[index] == separator)
		{
			string substring = src.substr(start, index - start);
			dest.push_back(substring);
			while (src[index + 1] == separator)
			{
				dest.push_back("");
				index++;
			}
			index++;
			start = index;
		}
		else
			index++;
	}
	//the last token
	string substring = src.substr(start);
	dest.push_back(substring);

}

void main()
{
	string csvPath = "F:\\FTP\\Data\\Trajectory\\logs_20090223_20090224.csv";
	ifstream ifs(csvPath);
	if (!ifs)
	{
		cout << "open " << csvPath << " error!" << endl;
		system("pause");
		exit(0);
	}
	string strLine, dummy;
	map<string, Traj*> cars;
	int count = 1;
	while (ifs)
	{
		if (count % 100000 == 0)
			cout << count << " ok" << endl;
		ifs >> dummy >> strLine;
		if (ifs.fail())
			break;
		vector<string> substrs;
		split(strLine, ',', substrs);
		//cout << dummy << " " << substrs[0] << " " <<  substrs[1] << " " << substrs[2] << " "<< substrs[3] << " " << substrs[4] << " " << substrs[5] << " " << substrs[6] << endl;
		//time
		int time = 3600 * ((substrs[0][0] - '0') * 10 + (substrs[0][1] - '0'))
			+ 60 * ((substrs[0][3] - '0') * 10 + (substrs[0][4] - '0'))
			+ ((substrs[0][6] - '0') * 10 + (substrs[0][7] - '0'));
		GeoPoint* pt = new GeoPoint(time, atof(substrs[4].c_str()), atof(substrs[3].c_str()));
		map<string, Traj*>::iterator iter = cars.find(substrs[1]);
		if (iter == cars.end())
		{
			Traj* traj = new Traj();
			traj->push_back(pt);
			cars.insert(make_pair(substrs[1], traj));
		}
		else
		{
			(*iter).second->push_back(pt);
		}
		count++;
	}
    //output
	ofstream ofs("out.txt");
	ofs << fixed << showpoint << setprecision(8);
	for each (pair<string, Traj*> item in cars)
	{
		for (Traj::iterator iter = item.second->begin(); iter != item.second->end(); iter++)
		{
			ofs << (*iter)->time << " " << (*iter)->lat << " " << (*iter)->lon << endl;
		}
		ofs << -1 << endl;		
	}
	ofs.close();
	ifs.close();
}

