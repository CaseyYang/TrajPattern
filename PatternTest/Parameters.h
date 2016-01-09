#pragma once

//NDBC工作所用到的参数
#define DE_DURATIVE 3 //序列最短长度
#define DE_MINSIMILARITY 0.2 //序列扩展条件：交集元素与并集元素之比
#define DE_MINOBJECTS 5 //路段聚类最少包含的轨迹数

//NDBC扩展所用到的参数
#define TIMECLUSTING_KMEANS_K 5 //时段聚类数量
#define TIMECLUSTERING_KMEANS_ITERTIME 15 //KMEANS迭代次数
#define TIMECLUSTERING_KMEANS_TESTTIME 20 //KMEANS随机选择初始点的次数

#define CGP_MINSIMILARITY 0.5 //粗粒度轨迹模式最小相似度
#define CGP_MINSUPPORT 10//粗粒度轨迹模式最小支持度