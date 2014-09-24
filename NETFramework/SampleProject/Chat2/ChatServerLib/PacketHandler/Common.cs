using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using CgsfNET64Lib;
using CGSFNETCommon;

namespace ChatServerLib.PacketHandler
{
    class Common : Base
    {
        public void SystemClientConnect(SFNETPacket packet)
        {
            var result = UserManagerRef.AddUser(packet.SessionID());
            InnerMessageQueue.CurrentUserCount(UserManagerRef.GetConnectCount());

            if (result == CSCommonLib.ERROR_CODE.NONE)
            {
                DevLog.Write(string.Format("Client Connect. SessionID: {0}", packet.SessionID()), LOG_LEVEL.INFO);
            }
            else
            {
                DevLog.Write(string.Format("Client Connect. SessionID: {0}, Error:{1}", packet.SessionID(), result.ToString()), LOG_LEVEL.ERROR);
            }
        }

        public void SystemClientDisConnected(SFNETPacket packet)
        {
            var result = UserManagerRef.RemoveUser(packet.SessionID());
            InnerMessageQueue.CurrentUserCount(UserManagerRef.GetConnectCount());

            if (result == CSCommonLib.ERROR_CODE.NONE)
            {
                DevLog.Write(string.Format("Client DisConnected. SessionID: {0}", packet.SessionID()), LOG_LEVEL.INFO);
            }
            else
            {
                DevLog.Write(string.Format("Client DisConnected. SessionID: {0}, Error:{1}", packet.SessionID(), result.ToString()), LOG_LEVEL.ERROR);
            }
        }
    }
}
