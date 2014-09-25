using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using CgsfNET64Lib;

namespace ChatServerLib
{
    class RelayPacketPacketProcess
    {
        static Action<short, SFNETPacket> RelayPacketFunc;


        public static bool SetFunction(Action<short, SFNETPacket> func)
        {
            if (RelayPacketFunc == null)
            {
                RelayPacketFunc = func;
                return true;
            }

            return false;
        }

        public static void RelayPacket(short lobbyID, SFNETPacket packet)
        {
            RelayPacketFunc(lobbyID, packet);
        }
    }
}
