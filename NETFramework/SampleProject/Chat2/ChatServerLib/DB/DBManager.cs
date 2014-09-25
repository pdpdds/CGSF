using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Runtime.CompilerServices;
using System.Collections.Concurrent;

using CSCommonLib;
using CGSFNETCommon;


namespace ChatServerLib.DB
{
    class DBManager
    {
        bool IsThreadRunning = false;
        List<System.Threading.Thread> ThreadList = new List<System.Threading.Thread>();

        ConcurrentQueue<RequestData> RequestQueue = new ConcurrentQueue<RequestData>();

        Dictionary<CSCommonLib.PACKET_ID, Func<RequestData, ResponseData>> DBWorkHandlerMap = new Dictionary<CSCommonLib.PACKET_ID, Func<RequestData, ResponseData>>();
        
        RequestHandler Handler = null;

        Action<ResponseData> DBResponseFunc = null;

        static Action<string, LOG_LEVEL, string, string, int> WriteLogToFileFunc = null;



        public ERROR_CODE CreateAndStart(int threadCount,
                                        Action<ResponseData> dbWorkResultFunc)
        {
            RegistPacketHandler();

            DBResponseFunc = dbWorkResultFunc;
            
            IsThreadRunning = true;

            for (int i = 0; i < threadCount; ++i)
            {
                var processThread = new System.Threading.Thread(this.Process);
                processThread.Start();

                ThreadList.Add(processThread);
            }

            return ERROR_CODE.NONE;
        }

        public void Destory()
        {
            IsThreadRunning = false;
        }

        public void InsertRequest(RequestData request)
        {
            RequestQueue.Enqueue(request);
        }


        Tuple<ERROR_CODE, string> RegistPacketHandler()
        {
            Handler = new RequestHandler();
            Handler.Init();


            DBWorkHandlerMap.Add(PACKET_ID.DB_REQUEST_LOGIN, Handler.RequestLogin);
                        

            return new Tuple<ERROR_CODE, string>(ERROR_CODE.NONE, "");
        }

        void Process()
        {
            RequestData request = null;

            while (IsThreadRunning)
            {
                try
                {
                    if (RequestQueue.TryDequeue(out request) == false)
                    {
                        System.Threading.Thread.Sleep(1);
                        continue;
                    }

                    if (DBWorkHandlerMap.ContainsKey(request.PacketID))
                    {
                        var result = DBWorkHandlerMap[request.PacketID](request);

                        if (result != null && result.PacketID != PACKET_ID.INVALID)
                        {
                            DBResponseFunc(result);
                        }
                    }
                    else
                    {
                        System.Diagnostics.Debug.WriteLine("세션 번호 {0}, DBWorkID {1}", request.SessionID, request.PacketID);
                    }
                }
                catch (Exception ex)
                {
                    DevLog.Write(ex.ToString(), LOG_LEVEL.ERROR);
                }
            }
        }


        public static void SetWriteFileLogFunction(Action<string, LOG_LEVEL, string, string, int> func)
        {
            WriteLogToFileFunc = func;
        }

        public static void WriteFileLog(string msg, LOG_LEVEL logLevel = LOG_LEVEL.TRACE,
                                [CallerFilePath] string fileName = "",
                                [CallerMemberName] string methodName = "",
                                [CallerLineNumber] int lineNumber = 0)
        {
            WriteLogToFileFunc(msg, logLevel, fileName, methodName, lineNumber);
        }
    }
}
