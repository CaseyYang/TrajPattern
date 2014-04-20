using System;
using System.Collections.Generic;
using System.IO;

namespace EdgePopularity
{
    class Program
    {
        static int CountEdges(string[] paths, Dictionary<int, int> edgesCounts)
        {
            int max = -1;
            foreach (string path in paths)
            {
                DirectoryInfo directory = new DirectoryInfo(path);
                FileInfo[] files = directory.GetFiles();
                foreach (FileInfo file in files)
                {
                    HashSet<int> curEdges = new HashSet<int>();
                    StreamReader fReader = new StreamReader(file.FullName);
                    while (!fReader.EndOfStream)
                    {
                        string[] rawStrs = fReader.ReadLine().Split(',');
                        int edge = Int32.Parse(rawStrs[1]);
                        if (edge != -1 && !curEdges.Contains(edge))
                        {
                            curEdges.Add(edge);
                            if (!edgesCounts.ContainsKey(edge))
                            {
                                edgesCounts.Add(edge, 1);
                            }
                            else
                            {
                                edgesCounts[edge]++;
                            }
                            if (max < edgesCounts[edge])
                            {
                                max = edgesCounts[edge];
                            }
                        }
                    }
                    fReader.Close();
                }
            }
            return max;
        }

        static Dictionary<int, double> GetEdgesPopularity(Dictionary<int, int> edgesCounts, int max)
        {
            Dictionary<int, double> result = new Dictionary<int, double>();
            foreach (KeyValuePair<int, int> pair in edgesCounts)
            {
                result.Add(pair.Key, (pair.Value + 0.0) / max * 100);
            }
            return result;
        }

        static void OutputEdgesPopularity(Dictionary<int, double> edgesPopularity, string fileName)
        {
            StreamWriter fWriter = new StreamWriter(fileName);
            fWriter.WriteLine(edgesPopularity.Count);
            foreach (KeyValuePair<int, double> pair in edgesPopularity)
            {
                fWriter.WriteLine(pair.Key + " " + pair.Value);
            }
            fWriter.Close();
        }

        static int MergeEdgeCounts(Dictionary<int, int> edgesCounts1, Dictionary<int, int> edgesCounts2, int max)
        {
            foreach (KeyValuePair<int, int> pair in edgesCounts1)
            {
                if (!edgesCounts2.ContainsKey(pair.Key))
                {
                    edgesCounts2.Add(pair.Key, pair.Value);
                }
                else
                {
                    edgesCounts2[pair.Key] += pair.Value;
                }
                if (max < edgesCounts2[pair.Key])
                {
                    max = edgesCounts2[pair.Key];
                }
            }
            return max;
        }

        static void Main(string[] args)
        {
            //读入7个文件夹中的地图匹配结果，得到每个路段对应的热度
            string[] weekendsPaths = { @"D:\MapMatchingProject\Data\新加坡数据\day1\day1_output", @"D:\MapMatchingProject\Data\新加坡数据\day2\day2_output" };
            string[] weekdaysPaths = { @"D:\MapMatchingProject\Data\新加坡数据\day3\day3_output", @"D:\MapMatchingProject\Data\新加坡数据\day4\day4_output", @"D:\MapMatchingProject\Data\新加坡数据\day5\day5_output", @"D:\MapMatchingProject\Data\新加坡数据\day6\day6_output", @"D:\MapMatchingProject\Data\新加坡数据\day7\day7_output" };

            //工作日：day3~day7
            Dictionary<int, int> weekdaysEdgesCounts = new Dictionary<int, int>();
            int weekdaysMax = CountEdges(weekdaysPaths, weekdaysEdgesCounts);
            Dictionary<int, double> weekdaysEdgesPopularity = GetEdgesPopularity(weekdaysEdgesCounts, weekdaysMax);
            OutputEdgesPopularity(weekdaysEdgesPopularity, "weekdaysEdgesPopularity.txt");
            Console.WriteLine("工作日部分处理完毕！");

            //周末：day1~day2
            Dictionary<int, int> weekendsEdgesCounts = new Dictionary<int, int>();
            int weekendsMax = CountEdges(weekendsPaths, weekendsEdgesCounts);
            Dictionary<int, double> weekendsEdgesPopularity = GetEdgesPopularity(weekendsEdgesCounts, weekendsMax);
            OutputEdgesPopularity(weekendsEdgesPopularity, "weekendsEdgesPopularity.txt");
            Console.WriteLine("周末部分处理完毕！");

            //合并
            weekendsMax = MergeEdgeCounts(weekdaysEdgesCounts, weekendsEdgesCounts, weekendsMax);
            Dictionary<int, double> totalEdgesPopularity = GetEdgesPopularity(weekendsEdgesCounts, weekendsMax);
            OutputEdgesPopularity(totalEdgesPopularity, "totalEdgesPopularity.txt");
        }
    }
}
