using UnityEngine;
using System.Collections;
using System.IO;
using System;

public class Cloger
{
    public enum E_LOGERTYPE
    {
        NONE, CONSOLE, FILE, BOTH
    }
    static private Cloger LoggerIns = null;
    E_LOGERTYPE eLogerType = E_LOGERTYPE.CONSOLE;
    string sFileName = "";
    StreamWriter sw = null;
    public Cloger()
    {
        initFileWrite();

    }
    void initFileWrite()
    {
        
        sFileName = DateTime.Today.DayOfYear.ToString() + DateTime.Today.Month.ToString() + DateTime.Today.Day.ToString() + DateTime.Today.Hour.ToString() + DateTime.Today.Minute.ToString() + ".txt";
        sw = new StreamWriter("./log/" + sFileName, false);
        sw.Write("");
        sw.Close();
    }
    void WriteFile(string s)
    {
        sw = new StreamWriter("./log/" + sFileName, true);
        sw.WriteLine(s);
        sw.Close();
    }
    void sendToLogView(string s)
    {
#if UNITY_EDITOR
        Debug.Log("LOG: " + s);
#else
            Console.WriteLine(s);
#endif
        DebugOverlay.Instance.AddViewportText(s,4);
    }
    public static void WR(string s,int iTime = 3, E_LOGERTYPE etype = E_LOGERTYPE.CONSOLE)
    {
        if (LoggerIns == null)
            LoggerIns = new Cloger();
        if (LoggerIns.eLogerType == E_LOGERTYPE.FILE)
        {
            LoggerIns.WriteFile(s);
        }
        else if (LoggerIns.eLogerType == E_LOGERTYPE.CONSOLE)
        {
            LoggerIns.sendToLogView(s);
        }
        else if (LoggerIns.eLogerType == E_LOGERTYPE.NONE)
        {
            
            LoggerIns.sendToLogView(s);
        }
        else
        {
            LoggerIns.WriteFile(s);
            LoggerIns.sendToLogView(s);
        }
    }
}
