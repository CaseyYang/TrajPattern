#pragma once
#include "FineGrainedPattern.h"

//�켣ģʽ����
class PatternCluster
{
public:
	list<FineGrainedPattern*> patterns;	//��ɹ켣ģʽ����ĸ����켣ģʽ
	int semanticType;					//�켣ģʽ�������������
	int startTimeStamp;					//�켣ģʽ������ʼʱ�����ʵ��ͳ���ã�
	int endTimeStamp;					//�켣ģʽ������ֹʱ�����ʵ��ͳ���ã�
	PatternCluster();
	void insertPattern(FineGrainedPattern* pattern);
	~PatternCluster();
};

