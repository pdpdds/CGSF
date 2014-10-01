using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using CGSFNETCommon;

namespace ChatClient1
{
    class PacketHandler
    {
        static MainForm MainFormRef;


        public static void Init(MainForm form)
        {
            MainFormRef = form;
        }

        public static void Process(JsonPacketData packet)
        {
            var packetType = (PACKET_ID)packet.PacketID;
            
            switch (packetType)
            {
                case PACKET_ID.SYSTEM_DISCONNECTD:
                    MainFormRef.SetDisconnectd();
                    break;

                case PACKET_ID.RESPONSE_LOGIN:
                    {
                        var resData = JsonEnDecode.Decode<JsonPacketResponseLogin>(packet.JsonFormatData);
                        var errorCode = (ERROR_CODE)resData.Result;
                        
                        if (errorCode == ERROR_CODE.NONE)
                        {
                            MainFormRef.SetClientStatus(CLIENT_STATUS.LOGIN);
                            DevLog.Write(string.Format("로그인 성공"), LOG_LEVEL.INFO);
                        }
                        else
                        {
                            DevLog.Write(string.Format("로그인 실패:{0}", errorCode.ToString()), LOG_LEVEL.ERROR);
                        }
                    }
                    break;
                    
                default:
                    break;
            }
        }
    }

    class JsonPacketData
    {
        public UInt16 PacketID;
        public UInt32 PacketOption;
        public UInt32 DataCRC;
        public UInt16 DataSize;
        public byte[] JsonFormatData;
    }
}
