using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CGSFNETCommon
{
    public class JsonEnDecode
    {
        public static byte[] Encode<T>(T data)
        {
            var jsonstring2 = Jil.JSON.Serialize<T>(data);
            var byteData = Encoding.UTF8.GetBytes(jsonstring2);
            return byteData;
        }

        public static T Decode<T>(byte[] jsonFormatData)
        {
            string jsonstring = System.Text.Encoding.GetEncoding("utf-8").GetString(jsonFormatData);
            var jsonData = Jil.JSON.Deserialize<T>(jsonstring);
            return jsonData;
        }
    }
}
