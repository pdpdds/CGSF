using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PacketEnDeCodeGeneratorLib
{
    public class CSharpPacketProtocolFileParse
    {
        public string FullPathFileName { get; private set; }
        public string FileName { get; private set; }
        string PacketDataNamePrefix = "PKT";

        public Dictionary<string, StructInfo> 클래스_정의_사전 = new Dictionary<string, StructInfo>();
        List<string> 클래스정의데이터 = new List<string>();
        List<Tuple<int, int>> 클래스_정의_위치_리스트 = new List<Tuple<int, int>>();


        public ERROR_CODE ParseFile(string fullPathFileName, string packetDataNamePrefix)
        {
            Clear();

            PacketDataNamePrefix = packetDataNamePrefix;
            FullPathFileName = fullPathFileName;
            FileName = System.IO.Path.GetFileName(fullPathFileName);

            var sr = LoadFile(FullPathFileName);
            if (sr == null)
            {
                return ERROR_CODE.ERROR_FILE_EXISTS;
            }

            var errorCode = 코드_대분류하기(sr);
            if (errorCode != ERROR_CODE.NONE)
            {
                return errorCode;
            }

            {
                var result = 클래스_정보_만들기(클래스정의데이터, 클래스_정의_위치_리스트);
                if (result.Item1 != ERROR_CODE.NONE)
                {
                    return result.Item1;
                }


                foreach (var classInfo in 클래스_정의_사전.Values)
                {
                    classInfo.DefineDataBufferSize += 클래스_데이터_크기_구하기(클래스_정의_사전, classInfo);
                }
            }

            return ERROR_CODE.NONE;
        }

        void Clear()
        {
            클래스_정의_사전.Clear();
            클래스정의데이터.Clear();
            클래스_정의_위치_리스트.Clear();
        }

        System.IO.StreamReader LoadFile(string fileName)
        {
            if (System.IO.File.Exists(fileName) == false)
            {
                return null;
            }

            var sr = new System.IO.StreamReader(fileName);
            return sr;
        }

        ERROR_CODE 코드_대분류하기(System.IO.StreamReader sr)
        {
            string readLine;
            List<int> 클래스_정의_시작위치_리스트 = new List<int>();

            while (sr.Peek() >= 0)
            {
                readLine = sr.ReadLine();

                if (string.IsNullOrEmpty(readLine))
                {
                    continue;
                }

                readLine = readLine.Trim();

                if (readLine.IndexOf("{") == 0 || readLine.IndexOf("}") == 0 || readLine.IndexOf("//") == 0)
                {
                    continue;
                }

                클래스정의데이터.Add(readLine);

                // 패킷 구조체 시작
                if (readLine.IndexOf("public class ") == 0)
                {
                    클래스_정의_시작위치_리스트.Add(클래스정의데이터.Count() - 1);
                }
            }


            클래스_정의_위치_분류(클래스_정의_시작위치_리스트, ref 클래스_정의_위치_리스트);
            
            return ERROR_CODE.NONE;
        }

        void 클래스_정의_위치_분류(List<int> 시작_위치_리스트, ref List<Tuple<int, int>> 정의_위치_리스트)
        {
            for (int i = 0; i < 시작_위치_리스트.Count(); ++i)
            {
                if (i == (시작_위치_리스트.Count() - 1))
                {
                    정의_위치_리스트.Add(new Tuple<int, int>(시작_위치_리스트[i], 클래스정의데이터.Count() - 1));
                }
                else
                {
                    정의_위치_리스트.Add(new Tuple<int, int>(시작_위치_리스트[i], 시작_위치_리스트[i + 1] - 1));
                }
            }
        }
                
        Tuple<ERROR_CODE, string> 클래스_정보_만들기(List<string> codeList, List<Tuple<int, int>> packetProtocolStartEndPos)
        {            
            foreach (var startEndPos in packetProtocolStartEndPos)
            {
                for (int i = startEndPos.Item1; i <= startEndPos.Item2; ++i)
                {
                    if (string.IsNullOrEmpty(codeList[i]))
                    {
                        continue;
                    }

                    var tokens = codeList[i].Split(new Char[] { ' ', '\t' }, StringSplitOptions.RemoveEmptyEntries);

                    if (tokens.Count() < 3)
                    {
                        return new Tuple<ERROR_CODE, string>(ERROR_CODE.CLASS_PARSE_STRUCT_TOKEN_COUNT_LESS, string.Format("코드:{0}, 토큰 개수{1}", codeList[i], tokens.Count()));
                    }

                    if (tokens[1] == "class")
                    {
                        bool isPassEnDecodeGenerate = false;
                        if (tokens[2].IndexOf(PacketDataNamePrefix) != 0)
                        {
                            isPassEnDecodeGenerate = true;
                        }

                        클래스_정의_사전.Add(tokens[2], 
                                        new StructInfo { Name = tokens[2],
                                               IsPassEnDecodeGenerate = isPassEnDecodeGenerate
                                        });
                    }
                    else
                    {
                        var memberInfo = 변수_타입_이름_분리하기(tokens);
                        var 구조체 = 클래스_정의_사전.Values.Last();
                        구조체.MemberList.Add(memberInfo);
                    }
                }
            }

            return new Tuple<ERROR_CODE, string>(ERROR_CODE.NONE, "성공");
        }
                
        MemberDataInfo 변수_타입_이름_분리하기(string[] tokens)
        {
            var member = new MemberDataInfo();

            if (tokens[1] == "const" )
            {
                if (tokens[2] == "unsigned")
                {
                    member.Type = tokens[2] + " " + tokens[3];
                    member.Name = tokens[4];
                }
                else
                {
                    member.Type = tokens[2];
                    member.Name = tokens[3];
                }
            }
            else if (tokens[1] == "unsigned")
            {
                member.Type = tokens[1] + " " + tokens[2];
                member.Name = tokens[3];
            }
            else
            {
                member.Type = tokens[1];
                member.Name = tokens[2];
            }

            if (member.Type.IndexOf("List<") >= 0)
            {
                int pos1 = member.Type.IndexOf("<");
                int pos2 = member.Type.IndexOf(">", pos1);
                member.Type = member.Type.Substring((pos1 + 1), (pos2 - pos1 - 1));
                member.ArrayValue = 8;
            }

            member.Name = member.Name.RemoveWhere(c => c == ';');
            
            return member;
        }

        short 클래스_데이터_크기_구하기(Dictionary<string, StructInfo> structMap, StructInfo structInfo)
        {
            short totalSize = 0;
            
            foreach (var memberInfo in structInfo.MemberList)
            {
                var typeSize = CheckDataType.CSharpTypeSize(memberInfo.Type);
                if (typeSize > 0)
                {
                    if (memberInfo.ArrayValue <= 1)
                    {
                        totalSize += typeSize;
                    }
                    else
                    {
                        return typeSize;
                    }
                }
                else
                {
                    if (structMap.ContainsKey(memberInfo.Type))
                    {
                        totalSize += 클래스_데이터_크기_구하기(structMap, structMap[memberInfo.Type]);
                    }
                    else
                    {
                        throw new System.Exception("클래스를 찾을 수 없습니다.");
                    }
                }
            }

            return totalSize;
        }
    }
}
