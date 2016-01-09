#pragma once
#include <iterator>
#include "FineGrainedPattern.h"

//�켣ģʽ����
class PatternCluster
{
public:
	set<FineGrainedPattern*> patterns;	//��ɹ켣ģʽ����ĸ����켣ģʽ
	int semanticType;					//�켣ģʽ�������������
	set<int> objs;						//�켣ģʽ��������Ĺ켣
	set<Edge*> edges;					//�켣ģʽ���������·��
	int startTimeStamp;					//�켣ģʽ������ʼʱ�����ʵ��ͳ���ã�
	int endTimeStamp;					//�켣ģʽ������ֹʱ�����ʵ��ͳ���ã�
	PatternCluster();
	PatternCluster(const PatternCluster& sourcePatternCluster, set<int>& trajObjs);//���ƹ��캯����ֻ����trajObjs���еĹ켣��ɵ�Pattern
	void insertPattern(FineGrainedPattern* pattern);
	~PatternCluster();
};

