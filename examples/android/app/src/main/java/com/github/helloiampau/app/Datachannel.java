package com.github.helloiampau.app;

import android.app.Activity;
import android.support.v7.app.AppCompatActivity;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import com.github.helloiampau.janus.generated.Bundle;
import com.github.helloiampau.janus.generated.JanusCommands;
import com.github.helloiampau.janus.generated.JanusEvent;
import com.github.helloiampau.janusclientsdk.JanusConfImpl;

import org.webrtc.DataChannel;

import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;

public class Datachannel extends AppCompatActivity implements ServiceDelegate {

  private JanusService _service;
  private DataChannel _datachannel;

  @Override
  protected void onCreate(android.os.Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.datachannel);

    Button start = this.findViewById(R.id.datachannel_test_start_button);
    Button stop = this.findViewById(R.id.datachannel_test_stop_button);
    Button send = this.findViewById(R.id.datachannel_test_send_button);
    EditText editor = this.findViewById(R.id.datachannel_test_message_edit);
    TextView statusView = this.findViewById(R.id.datachannel_test_status_text);
    TextView log = this.findViewById(R.id.datachannel_test_messages);

    String host = this.getIntent().getStringExtra("HOST");
    if(host == null) {
      host = "http://10.0.2.2:8088/janus";
    }

    JanusConfImpl conf = new JanusConfImpl();
    conf.url(host);
    conf.plugin("janus.plugin.echotest");

    this._service = new JanusService(this, this, conf);
    this._service.statusListener().observe(this, status -> {
      if(status.equals("READY")) {
        Bundle payload = Bundle.create();
        payload.setBool("audio", false);
        payload.setBool("video", false);

        this._service.dispatch(JanusCommands.CALL, payload);
      } else if(status.equals("OFF") || status.equals("CLOSED")) {
        start.setEnabled(true);
        stop.setEnabled(false);
        send.setEnabled(false);
        editor.setEnabled(false);
      }

      statusView.setText(status);
    });

    this._service.dataChannelListener().observe(this, dataChannel -> {
      if(dataChannel == null) {
        start.setEnabled(true);
        stop.setEnabled(false);
        send.setEnabled(false);
        editor.setEnabled(false);
      } else {
        start.setEnabled(false);
        stop.setEnabled(true);
        send.setEnabled(true);
        editor.setEnabled(true);
      }

      this._datachannel = dataChannel;
    });

    stop.setOnClickListener(v -> {
      stop.setEnabled(false);
      start.setEnabled(false);
      send.setEnabled(false);
      editor.setEnabled(false);
      this._service.stop();
    });

    start.setOnClickListener(v -> {
      stop.setEnabled(false);
      start.setEnabled(false);
      send.setEnabled(false);
      editor.setEnabled(false);
      this._service.start();
    });

    this._service.logListener().observe(this, text -> {
      if(text == null) {
        return;
      }

      log.setText(text);
    });

    send.setOnClickListener(v -> {
      String text = editor.getText().toString();
      editor.setText("");
      InputMethodManager imm = (InputMethodManager) this.getSystemService(Activity.INPUT_METHOD_SERVICE);
      imm.hideSoftInputFromWindow(editor.getWindowToken(), 0);

      if(text.length() == 0) {
        return;
      }

      try {
        ByteBuffer data = ByteBuffer.wrap(text.getBytes("UTF-8"));
        DataChannel.Buffer buffer = new DataChannel.Buffer(data, false);
        this._datachannel.send(buffer);
      } catch (UnsupportedEncodingException e) {
        e.printStackTrace();
      }
    });
  }

  @Override
  public void onEvent(JanusEvent event, Bundle payload) {

  }

}
