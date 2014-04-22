package com.cgsf.chatclientandroid;

import android.app.Activity;
import android.content.Context;
import android.graphics.Typeface;
import android.os.Bundle;
import android.os.Handler;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ScrollView;
import android.widget.TextView;
import net.alhem.jsockets.SocketHandler;
import net.alhem.jsockets.StdLog;
import net.alhem.jsockets.StdoutLog;

import java.util.Queue;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ConcurrentLinkedQueue;

public class ChatActivity extends Activity implements View.OnClickListener
{
    Queue<String> queue = new ConcurrentLinkedQueue<String>();
    ChatClient chatClient = null;
    Thread chatThread = null;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        Button button = (Button)findViewById(R.id.connectbutton);
        Button disconnectButton = (Button)findViewById(R.id.disconnectbutton);
        Button sendButton = (Button)findViewById(R.id.sendbutton);
        Button exitButton = (Button)findViewById(R.id.exitbutton);

        button.setOnClickListener(this);
        disconnectButton.setOnClickListener(this);
        sendButton.setOnClickListener(this);
        exitButton.setOnClickListener(this);

        TextView chatView = (TextView)findViewById(R.id.textView);

        chatView.append("CGSF 채팅 클라이언트 안드로이드\n");
    }

    @Override
    public void onClick(View v) {
        //To change body of implemented methods use File | Settings | File Templates.
        switch(v.getId()){

            case R.id.connectbutton:

                Button button = (Button)findViewById(R.id.connectbutton);
                button.setEnabled(false);

                EditText ipAddress = (EditText)findViewById(R.id.ipaddress);
                EditText portText = (EditText)findViewById(R.id.port);

                String szIP = ipAddress.getText().toString();
                int port = Integer.parseInt(portText.getText().toString(), 10);

                chatClient = new ChatClient(this, queue, szIP, port);
                chatThread = new Thread(chatClient);

                chatThread.start();

                break;

            case R.id.sendbutton:
                EditText chatText = (EditText)findViewById(R.id.chatmessage);

                String szChat = chatText.getText().toString();

                if(szChat.length() == 0)
                    break;

                if(chatClient != null)
                {
                    queue.offer(szChat);
                }

                InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(chatText.getWindowToken(), 0);

                break;

            case R.id.disconnectbutton:
                Button connectButton = (Button)findViewById(R.id.connectbutton);
                connectButton.setEnabled(true);

                if(chatClient != null)
                    chatClient.setStop();

                break;

            case R.id.exitbutton:
                System.exit(0);
                break;
        }
    }

    private Handler mHandler = new Handler();

    // This gets executed in a non-UI thread:
    public void appendChatMessage(String szChatMessage) {

        final String str = szChatMessage;
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                // This gets executed on the UI thread so it can safely modify Views
                TextView chatView = (TextView)findViewById(R.id.textView);
                chatView.append(str);
            }
        });
    }
}
