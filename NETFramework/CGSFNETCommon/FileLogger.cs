using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CGSFNETCommon
{
    public class FileLogger
    {
        public static Logger Logger = LogManager.GetLogger(DateTime.Now.ToString("yyyyMMddHHmm"));


        public static void Debug(string message)
        {
            Logger.Debug(message);
        }

        public static void Info(string message)
        {
            Logger.Info(message);
        }

        public static void Error(string message)
        {
            Logger.Error(message);
        }
    }
}
