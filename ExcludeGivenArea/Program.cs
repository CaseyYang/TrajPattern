using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ExcludeGivenArea
{
    class Program
    {
        static void Main(string[] args)
        {
            List<string> validInputFiles = new List<string>();
            List<string> validAnswerFiles = new List<string>();
            for (int i = 1; i <= 9; ++i)
            {
                DirectoryInfo inputDirectory = new DirectoryInfo(@"D:\MapMatchingProject\Data\新加坡数据\day" + i + @"\day" + i + @"_splited_input\");
                DirectoryInfo answerDirectory = new DirectoryInfo(@"D:\MapMatchingProject\Data\新加坡数据\day" + i + @"\day" + i + @"_splited_answer\");
                var inputFiles = inputDirectory.GetFiles();
                var answerFiles = answerDirectory.GetFiles();
                for (int j = 0; j < inputFiles.Length; ++j)
                {
                    var file = inputFiles[j];
                    bool validFlag = true;
                    StreamReader fReader = new StreamReader(file.FullName);
                    while (!fReader.EndOfStream)
                    {
                        string rawStr = fReader.ReadLine();
                        string[] rawStrs = rawStr.Split(',');
                        double latitude = Double.Parse(rawStrs[1]);
                        double longitude = Double.Parse(rawStrs[2]);
                        if (latitude > 1.3393 && latitude < 1.3838 && longitude > 103.9755 && longitude < 104.0004)
                        {
                            validFlag = false;
                            break;
                        }
                    }
                    fReader.Close();
                    if (validFlag)
                    {
                        validInputFiles.Add(file.FullName);
                        validAnswerFiles.Add(answerFiles[j].FullName);
                    }
                }
                Console.WriteLine("文件夹" + i + "中的轨迹文件读入完毕！");
            }
            for (int index = 0; index < validInputFiles.Count; ++index)
            {
                FileInfo file = new FileInfo(validAnswerFiles[index]);
                file.CopyTo(@"D:\MapMatchingProject\Data\新加坡数据\9daysForTrajPattern\answer\output_" + index + ".txt");
                file = new FileInfo(validInputFiles[index]);
                file.CopyTo(@"D:\MapMatchingProject\Data\新加坡数据\9daysForTrajPattern\input\input_" + index + ".txt");
            }
            Console.WriteLine("所有匹配结果文件复制完毕！");
            Console.WriteLine("按任意键继续……");
            Console.Read();
        }
    }
}
