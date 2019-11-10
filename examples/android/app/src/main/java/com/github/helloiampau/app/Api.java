package com.github.helloiampau.app;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.widget.Button;
import android.widget.TextView;

import com.github.helloiampau.janus.generated.JanusEvent;
import com.github.helloiampau.janusclientsdk.JanusConfImpl;

public class Api extends AppCompatActivity implements ServiceDelegate {

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.api);

    TextView statusView = this.findViewById(R.id.api_test_status_text);
    Button stop = this.findViewById(R.id.api_test_stop_button);
    Button start = this.findViewById(R.id.api_test_start_button);

    String host = this.getIntent().getStringExtra("HOST");
    if(host == null) {
      host = "http://10.0.2.2:8088/janus";
    }

    JanusConfImpl conf = new JanusConfImpl();
    conf.url(host);
    conf.plugin("janus.plugin.echotest");

    JanusService service = new JanusService(this, this, conf);
    service.statusListener().observe(this, status -> {
      if(status.equals("READY")) {
        stop.setEnabled(true);
        start.setEnabled(false);
      } else if(status.equals("OFF") || status.equals("CLOSED")) {
        start.setEnabled(true);
        stop.setEnabled(false);
      }

      statusView.setText(status);
    });

    start.setOnClickListener(v -> {
      stop.setEnabled(false);
      start.setEnabled(false);
      service.start();
    });

    stop.setOnClickListener(v ->{
      stop.setEnabled(false);
      start.setEnabled(false);
      service.stop();
    });
  }

  @Override
  public void onEvent(JanusEvent event, com.github.helloiampau.janus.generated.Bundle payload) {

  }
}
