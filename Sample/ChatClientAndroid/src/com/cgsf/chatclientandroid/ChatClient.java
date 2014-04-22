package com.cgsf.chatclientandroid;

import net.alhem.jsockets.SocketHandler;
import net.alhem.jsockets.StdLog;
import net.alhem.jsockets.StdoutLog;
import org.json.simple.JSONObject;

import java.util.Queue;

/**
 * Created with IntelliJ IDEA.
 * User: darkx
 * Date: 14. 4. 22
 * Time: 오후 5:08
 * To change this template use File | Settings | File Templates.
 */
public class ChatClient implements Runnable {

    final static short CHAT_PACKET_NUM = 1000;
    ChatActivity activity = null;
    Queue<String> queue = null;
    String szIP;
    int port;
    boolean isStop = false;

    void setStop()
    {
        isStop = true;
    }

    void ProcessChatMessage(String szMessage)
    {
        activity.appendChatMessage(szMessage);
    }

    public ChatClient(ChatActivity activity, Queue<String> queue, String szIP, int port)
    {
        this.activity = activity;
        this.queue = queue;
        this.szIP = szIP;
        this.port = port;
    }

    public void run() {
        StdLog log = new StdoutLog();
        SocketHandler h = new SocketHandler(log);
        ChatClientSocket chatSocket = new  ChatClientSocket(h);
        chatSocket.SetOwner(this);
        chatSocket.Open( szIP, port );
        h.Add( chatSocket );
        h.Select(1, 0);

        while (isStop == false) // forever
        {
            h.Select(1, 0);
            String szChatMessage = queue.poll();
            if(szChatMessage != null)
            {
                JSONObject obj = new JSONObject();
                obj.put("chat", szChatMessage);

                chatSocket.SendPacket(CHAT_PACKET_NUM, obj);
            }
        }
    }
}
