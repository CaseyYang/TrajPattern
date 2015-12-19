#pragma once
#include <vector>
#include "FineGrainedPattern.h"
using namespace std;

//ʱ������㷨�е�һ��ʱ���
class PatternTimeSlot
{
public:
	vector<FineGrainedPattern*> patterns;	//ʱ���������Ԫ�ض�Ӧ��ϸ���ȹ켣ģʽ
	vector<int> timeStamps;					//ÿ���켣ģʽ��ʱ����ֵ��Ϊʱ����е�һ��Ԫ��
	int center;								//ʱ�������
	double SSE;
	PatternTimeSlot();
	PatternTimeSlot(list<FineGrainedPattern*>&patterns);
	void insertPattern(FineGrainedPattern* pattern);	//��ʱ���������һ��Ԫ��
	double calcSSE();									//����ʱ��ص�SSE�����ƽ���ͣ�
	~PatternTimeSlot();
};

