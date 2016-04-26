#include "PatternTimeSlot.h"


PatternTimeSlot::PatternTimeSlot() :patterns(vector<FineGrainedPattern*>()), timeStamps(vector<int>()), center(0), SSE(0)
{
}

PatternTimeSlot::PatternTimeSlot(list<FineGrainedPattern*>&patterns) : patterns(patterns.begin(), patterns.end()), timeStamps(vector<int>()), center(0), SSE(0) {
	for each (FineGrainedPattern* pattern in this->patterns)
	{
		int avg = (pattern->startTime + pattern->endTime) / 2;
		timeStamps.push_back(avg);
		center += avg;
	}
	center /= static_cast<int>(timeStamps.size());
}

void PatternTimeSlot::insertPattern(FineGrainedPattern* pattern) {
	patterns.push_back(pattern);
	int avg = (pattern->startTime + pattern->endTime) / 2;
	timeStamps.push_back(avg);
	//不能增量式更新center，因为这样每次更新都会有进度损失（center是int型），在KMEANS算法中经过几百次增量式更新其误差会达到非常离谱的程度
	//center = (center*(static_cast<int>(timeStamps.size()) - 1) + avg) / static_cast<int>(timeStamps.size());
}

int PatternTimeSlot::calcCenter()
{
	if (center != 0) {
		cout << "在计算时间簇时间中心时出错！" << endl;
	}
	for each (int timeStamp in this->timeStamps)
	{
		center += timeStamp;
	}
	center/= static_cast<int>(timeStamps.size());
	return center;
}

double PatternTimeSlot::calcSSE() {
	this->SSE = 0;
	for each (int timeStamp in this->timeStamps)
	{
		this->SSE += pow(timeStamp - center, 2);
	}
	return this->SSE;
}

void PatternTimeSlot::clear()
{
	patterns.clear();
	timeStamps.clear();
	patternClusters.clear();
	center = 0;
	SSE = 0;
}

void PatternTimeSlot::outputTimeStamps(string filePath) {
	ofstream fout(filePath);
	for each (int timeStamp in this->timeStamps)
	{
		fout << timeStamp << endl;
	}
	fout.close();
}

PatternTimeSlot::~PatternTimeSlot()
{
}
