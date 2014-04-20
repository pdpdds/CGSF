package com.example.chatclientandroid;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import net.alhem.jsockets.SocketHandler;
import net.alhem.jsockets.StdLog;
import net.alhem.jsockets.StdoutLog;

public class ChatActivity extends Activity implements View.OnClickListener
{
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        Button button = (Button)findViewById(R.id.connectbutton);

        button.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        //To change body of implemented methods use File | Settings | File Templates.
        switch(v.getId()){

            case R.id.connectbutton:
                Button button = (Button)findViewById(R.id.connectbutton);
                button.setEnabled(false);;

                StdLog log = new StdoutLog();
                SocketHandler h = new SocketHandler(log);
                ChatClientSocket chatSocket = new  ChatClientSocket(h, this);
                chatSocket.Open( "127.0.0.1", 25251 );
                h.Add( chatSocket );

                while (!chatSocket.quit) // forever
                {
                    h.Select(1, 0);
                }
            case R.id.chatmessage:
                //ProcessChat();

                break;
        }
    }
}
