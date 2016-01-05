#pragma once
#include <vector>
#include "FineGrainedPattern.h"
#include "PatternCluster.h"
using namespace std;

//ʱ������㷨�е�һ��ʱ���
class PatternTimeSlot
{
public:
	vector<FineGrainedPattern*> patterns;	//ʱ���������Ԫ�ض�Ӧ��ϸ���ȹ켣ģʽ
	vector<int> timeStamps;					//ÿ���켣ģʽ��ʱ����ֵ��Ϊʱ����е�һ��Ԫ��
	int center;								//ʱ�������
	double SSE;
	list<PatternCluster*> patternClusters;	//ʱ����������Ĺ켣ģʽ����
	PatternTimeSlot();
	PatternTimeSlot(list<FineGrainedPattern*>&patterns);
	void insertPattern(FineGrainedPattern* pattern);	//��ʱ���������һ��Ԫ��
	double calcSSE();									//����ʱ��ص�SSE�����ƽ���ͣ�
	void clear();										//���ʱ�������������
	void outputTimeStamps(string filePath);				//������е���ֵʱ�����������
	~PatternTimeSlot();
};

