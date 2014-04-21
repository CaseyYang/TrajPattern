using System;
using System.IO;

namespace MergeTrajsAndMatchedEdges
{
    class Program
    {
        static string basePath = @"D:\MapMatchingProject\Data\新加坡数据\day";
        static string trajDirectoryStr = "_input";
        static string matchedEdgeDirectoryStr = "_new";
        static void MergeTrajAndMatchedEdgesFiles(int i)
        {
            DirectoryInfo trajDirectory = new DirectoryInfo(basePath + i.ToString() + @"\day" + i.ToString() + trajDirectoryStr);
            FileInfo[] files = trajDirectory.GetFiles();
            foreach (var file in files)
            {
                string trajFileName = file.FullName;
                string matchedFileName = trajFileName.Replace("input", "output");
                StreamReader trajFileReader = new StreamReader(trajFileName);
                StreamReader matchedResultReader = new StreamReader(matchedFileName);
                StreamWriter fWriter = new StreamWriter(basePath + i.ToString() + @"\day" + i.ToString() + matchedEdgeDirectoryStr + "/" + file.Name);
                while (!trajFileReader.EndOfStream)
                {
                    string rawStrFromTraj = trajFileReader.ReadLine();
                    string[] rawStrsFromMatchedEdges = matchedResultReader.ReadLine().Split(',');
                    rawStrFromTraj += "," + rawStrsFromMatchedEdges[1];
                    fWriter.WriteLine(rawStrFromTraj);
                }
                trajFileReader.Close();
                matchedResultReader.Close();
                fWriter.Close();
            }
        }
        static void Main(string[] args)
        {
            int[] num = { 1, 2, 3, 4, 5, 6, 7 };
            for (int i = 0; i < num.Length; i++)
            {
                MergeTrajAndMatchedEdgesFiles(num[i]);
                Console.WriteLine("day" + num[i] + "处理完毕!");
            }
        }
    }
}
