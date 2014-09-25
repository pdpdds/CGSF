using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using MsgPack.Serialization;
using CSCommonLib;


namespace ChatServerLib.DB
{
    class RequestHandler
    {        
        public void Init()
        {
        }

        public ResponseData RequestLogin(RequestData dbRequest)
        {
            try
            {
                var serializer = MessagePackSerializer.Get<DB.RequestLogin>();
                var request = serializer.UnpackSingleObject(dbRequest.Datas);

                return RequestLoginValue(ERROR_CODE.NONE, dbRequest);
            }
            catch
            {
                return RequestLoginValue(ERROR_CODE.DB_LOGIN_EXCEPTION, dbRequest);
            }
        }

        ResponseData RequestLoginValue(ERROR_CODE result, RequestData dbRequest)
        {
            var reqponseData = new ResponseData()
            {
                PacketID = PACKET_ID.DB_RESPONSE_LOGIN,
                SessionID = dbRequest.SessionID,
                PacketProcessIndex = dbRequest.PacketProcessIndex,
            };

            var response = new DB.ResponseLogin() { Result = result, UserID = dbRequest.UserID };
            var serializer = MessagePackSerializer.Get<DB.ResponseLogin>();
            reqponseData.Datas = serializer.PackSingleObject(response);

            return reqponseData;
        }
    }
}
