package com.cgsf.chatclientandroid;

import android.app.Activity;
import android.widget.TextView;
import android.widget.Toast;
import net.alhem.jsockets.SocketHandler;
import net.alhem.jsockets.TcpSocket;

import java.io.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Queue;

import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;

public class ChatClientSocket extends TcpSocket
{
    final static short CHAT_PACKET_HEDER_SIZE = 12;

    ByteBuffer m_ioBuffer = ByteBuffer.allocate(4096);
    JSONParser parser = new JSONParser();
    Queue<String> queue = null;
    ChatClient chatClient = null;

    public void SetQueue(Queue<String> queue)
    {
        this.queue = queue;
    }

    public void SetOwner(ChatClient chatClient)
    {
        this.chatClient = chatClient;


    }

    public static void writeLittleEndianInteger(int i, OutputStream ops)
            throws IOException {
        byte[] buffer = new byte[4];
        buffer[0] = (byte) i;
        buffer[1] = (byte) (i >> 8);
        buffer[2] = (byte) (i >> 16);
        buffer[3] = (byte) (i >> 24);
        ops.write(buffer);
    }

    public static void writeLittleEndianShort(short i, OutputStream ops)
            throws IOException {
        byte[] buffer = new byte[2];
        buffer[0] = (byte) i;
        buffer[1] = (byte) (i >> 8);
        ops.write(buffer);
    }

    public ChatClientSocket(SocketHandler h )
    {
        super(h);
        SetLineProtocol(false);
      //  this.activity = activity;
    }

    public void OnConnect()
    {
        chatClient.ProcessChatMessage("서버와 연결되었습니다\n");

        JSONObject obj = new JSONObject();
        obj.put("chat", "박주항");

        SendPacket((short)1000, obj);
    }

    public void SendPacket(short packetID, JSONObject obj)
    {
        ByteArrayOutputStream baos = null;
        DataOutputStream dos = null;

        try{
            baos = new ByteArrayOutputStream();
            dos = new DataOutputStream(baos);

            writeLittleEndianShort(packetID, dos);
            writeLittleEndianInteger(0, dos);
            writeLittleEndianInteger(0, dos);

           String szStr = obj.toString();
            byte [] utf8 = szStr.getBytes("UTF-16LE");

            writeLittleEndianShort((short)utf8.length, dos);

            dos.write(utf8, 0, (short)utf8.length);
            dos.flush();

            SendBuf(baos.toByteArray(), baos.size());

            // for each byte in the buffer
        }catch(Exception e){
            // if any I/O error occurs
            e.printStackTrace();

        }finally{

            // releases all system resources from the streams
            if(dos!=null)
                try {
                    dos.close();
                } catch (IOException e) {
                    e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
                }
            if(baos!=null)
                try {
                    baos.close();
                } catch (IOException e) {
                    e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
                }
        }

    }

    public void OnRawData(java.nio.ByteBuffer b, int len) {
    /* compiled code */
        m_ioBuffer.order(ByteOrder.LITTLE_ENDIAN);
        m_ioBuffer.put(b);

        while(true)
        {
        m_ioBuffer.flip();

        int bufferLimit = m_ioBuffer.limit();
        if(bufferLimit < CHAT_PACKET_HEDER_SIZE)
        {
            m_ioBuffer.clear();
            return;
        }
        //m_ioBuffer.get(header, 0, CHAT_PACKET_HEDER_SIZE);
        short packetid = m_ioBuffer.getShort();
        int dataCrc = m_ioBuffer.getInt();
        int packetOption = m_ioBuffer.getInt();
        short dataSize = m_ioBuffer.getShort();

        if(bufferLimit < dataSize + CHAT_PACKET_HEDER_SIZE)
        {
            m_ioBuffer.clear();
            return;
        }

        byte [] data = new byte [dataSize];

        m_ioBuffer.get(data, 0, dataSize);
        String str  = new String(data,0,data.length);

        try {
            JSONObject obj = (JSONObject)parser.parse(str);
            String chat = (String)obj.get("chat");
            String who = (String)obj.get("who");

            System.out.println(who + " : " + chat);

            chatClient.ProcessChatMessage(who + " : " + chat + "\n");

        } catch (ParseException e) {
            e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
        }

        m_ioBuffer.compact();
        }
    }
}