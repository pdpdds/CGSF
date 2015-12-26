using UnityEngine;
using System.Collections;
using System;
using System.Text;
using Newtonsoft.Json;
namespace System.Runtime.CompilerServices
{
    public class ExtensionAttribute : Attribute { }
}
namespace NETCODE
{
    public class PacketHeader
    {//12
        public UInt16 Protocol;//2
        public UInt32 PacketOption;//4
        public UInt32 DataCRC;//4
        public UInt16 DataSize;//2
        public const int HEADERSIZE = 12;
    }
    public class jPacket : PacketHeader
    {
        public byte[] JsonFormatData = null;
        public const int PACKETSIZE = 8096;

        public jPacket() 
        {//Receive 전용@
        }
        public jPacket(UInt16 uiPrID)
        {//Send 전용@
            Protocol = uiPrID;
        }
        public static T MakeObject<T>(byte[] bt)
        {
            string sJson = Encoding.UTF8.GetString(bt);
            T data = Newtonsoft.Json.JsonConvert.DeserializeObject<T>(sJson);
            return data;
        }
        public byte[] MakePacket<T>(T data)
        {
            string sJson = Newtonsoft.Json.JsonConvert.SerializeObject(data);
            JsonFormatData = Encoding.UTF8.GetBytes(sJson);
            System.Collections.Generic.List<byte> btPacketSource = new System.Collections.Generic.List<byte>();

            btPacketSource.AddRange(BitConverter.GetBytes(Protocol));//protocol 2
            btPacketSource.AddRange(BitConverter.GetBytes((UInt32)0));//PacketOption 4
            btPacketSource.AddRange(BitConverter.GetBytes((UInt32)0));//DataCRC 4
            btPacketSource.AddRange(BitConverter.GetBytes((UInt16)JsonFormatData.Length));//DataSize 2
            btPacketSource.AddRange(JsonFormatData);//data
            return btPacketSource.ToArray();
        }
        
    }
}