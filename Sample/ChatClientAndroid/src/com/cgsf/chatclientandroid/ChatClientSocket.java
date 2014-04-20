package com.example.chatclientandroid;

import android.app.Activity;
import android.widget.Toast;
import net.alhem.jsockets.SocketHandler;
import net.alhem.jsockets.TcpSocket;

import java.io.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;

public class ChatClientSocket extends TcpSocket
{
    final static short CHAT_PACKET_ID = 1000;
    final static short CHAT_PACKET_HEDER_SIZE = 12;
    public boolean quit = false;
    ByteBuffer m_ioBuffer = ByteBuffer.allocate(4096);
    JSONParser parser = new JSONParser();
    ChatActivity activity;

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

    public ChatClientSocket(SocketHandler h, ChatActivity activity)
    {
        super(h);
        SetLineProtocol(false);
        this.activity = activity;
    }

    public void OnConnect()
    {
        JSONObject obj = new JSONObject();
        obj.put("chat", "hi cgsf chatting server!!");

        SendPacket(CHAT_PACKET_ID, obj);

        String msg = "kkkkkkkkkkkkkk";
        Toast.makeText(activity, msg, Toast.LENGTH_SHORT).show();

    }

    public void SendPacket(short packetID, JSONObject obj)
    {
        ByteArrayOutputStream baos = null;
        DataOutputStream dos = null;

        try{
            baos = new ByteArrayOutputStream();
            dos = new DataOutputStream(baos);

            dos.writeInt(packetID);
            writeLittleEndianShort(packetID, dos);
            writeLittleEndianInteger(0, dos);
            writeLittleEndianInteger(0, dos);
            writeLittleEndianShort((short)obj.toJSONString().length(), dos);
            dos.write(obj.toJSONString().getBytes(), 0, obj.toJSONString().length());
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

        } catch (ParseException e) {
            e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
        }

        m_ioBuffer.compact();
        }
    }

    public void OnDelete() {
    /* compiled code */
        quit = true;
    }
}