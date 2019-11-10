package com.github.helloiampau.app;

import android.content.Context;
import android.media.AudioManager;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.Button;
import android.widget.TextView;

import com.github.helloiampau.janus.generated.JanusCommands;
import com.github.helloiampau.janus.generated.JanusEvent;
import com.github.helloiampau.janusclientsdk.JanusConfImpl;

import org.webrtc.RendererCommon;
import org.webrtc.SurfaceViewRenderer;

public class Echotest extends AppCompatActivity implements ServiceDelegate {

  private JanusService _service;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.echotest);

    TextView statusView = this.findViewById(R.id.echotest_test_status_text);
    Button stop = this.findViewById(R.id.echotest_test_stop_button);
    Button start = this.findViewById(R.id.echotest_test_start_button);
    Button call = this.findViewById(R.id.echotest_test_call_button);
    Button hangup = this.findViewById(R.id.echotest_test_hangup_button);
    SurfaceViewRenderer localView = this.findViewById(R.id.echotest_local_view);
    SurfaceViewRenderer remoteView = this.findViewById(R.id.echotest_remote_view);

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
        stop.setEnabled(true);
        start.setEnabled(false);
        call.setEnabled(true);
        hangup.setEnabled(true);
      } else if(status.equals("OFF") || status.equals("CLOSED")) {
        start.setEnabled(true);
        stop.setEnabled(false);
        call.setEnabled(false);
        hangup.setEnabled(false);
      }

      localView.release();
      remoteView.release();

      statusView.setText(status);
    });

    this._service.localVideoTrackListener().observe(this, videoTrack -> {
      if(videoTrack == null) {
        return;
      }

      localView.init(this._service.rootEgl().getEglBaseContext(), null);
      localView.setScalingType(RendererCommon.ScalingType.SCALE_ASPECT_FILL);
      videoTrack.addSink(localView);
    });

    this._service.remoteVideoTrackListener().observe(this, videoTrack -> {
      if(videoTrack == null) {
        return;
      }

      remoteView.init(this._service.rootEgl().getEglBaseContext(), null);
      remoteView.setScalingType(RendererCommon.ScalingType.SCALE_ASPECT_FILL);
      videoTrack.addSink(remoteView);

      hangup.setEnabled(true);
      stop.setEnabled(true);
    });

    this._service.remoteAudioTrackListener().observe(this, audioTrack -> {
      if(audioTrack == null) {
        return;
      }

      AudioManager audioManager = ((AudioManager) this.getSystemService(Context.AUDIO_SERVICE));
      audioManager.setMode(AudioManager.MODE_IN_COMMUNICATION);
      audioManager.setSpeakerphoneOn(true);
    });

    this._service.errorListener().observe(this, error -> {
      Log.v("janus-client", error.getMessage());
    });

    start.setOnClickListener(v -> {
      stop.setEnabled(false);
      start.setEnabled(false);
      call.setEnabled(false);
      hangup.setEnabled(false);
      this._service.start();
    });

    stop.setOnClickListener(v -> {
      stop.setEnabled(false);
      start.setEnabled(false);
      call.setEnabled(false);
      hangup.setEnabled(false);
      this._service.stop();
    });

    call.setOnClickListener(v -> {
      stop.setEnabled(false);
      start.setEnabled(false);
      call.setEnabled(false);
      hangup.setEnabled(false);

      com.github.helloiampau.janus.generated.Bundle bundle = com.github.helloiampau.janus.generated.Bundle.create();
      this._service.dispatch(JanusCommands.CALL, bundle);
    });

    hangup.setOnClickListener(v -> {
      stop.setEnabled(false);
      start.setEnabled(false);
      call.setEnabled(false);
      hangup.setEnabled(false);

      this._service.hangup();
    });
  }

  @Override
  public void onEvent(JanusEvent event, com.github.helloiampau.janus.generated.Bundle payload) {

  }
}
