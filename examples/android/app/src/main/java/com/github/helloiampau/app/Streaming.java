package com.github.helloiampau.app;

import android.content.Context;
import android.media.AudioManager;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;

import com.github.helloiampau.janus.generated.Bundle;
import com.github.helloiampau.janus.generated.JanusCommands;
import com.github.helloiampau.janus.generated.JanusData;
import com.github.helloiampau.janus.generated.JanusEvent;
import com.github.helloiampau.janus.generated.JanusPlugins;
import com.github.helloiampau.janusclientsdk.JanusConfImpl;

import org.webrtc.RendererCommon;
import org.webrtc.SurfaceViewRenderer;

import java.util.ArrayList;
import java.util.List;

public class Streaming extends AppCompatActivity implements ServiceDelegate {

  private Button _stop;
  private Button _start;

  class Description {
    public long id;
    public String description;

    public Description(JanusData data) {
      this.id = data.getInt("id", -1);
      this.description = data.getString("description", "");
    }
  }

  private List<Description> _descriptions = new ArrayList<>();
  private BaseAdapter _listAdapter;
  private TextView _streaming;

  @Override
  protected void onCreate(android.os.Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.streaming);

    TextView statusView = this.findViewById(R.id.streaming_test_status_text);
    this._stop = this.findViewById(R.id.streaming_test_stop_button);
    this._streaming = this.findViewById(R.id.streaming_test_status);
    this._streaming.setText("streaming: false");
    this._start = this.findViewById(R.id.streaming_test_start_button);
    ListView list = this.findViewById(R.id.streaming_test_feed_list);
    SurfaceViewRenderer videoView = this.findViewById(R.id.streaming_test_video);

    String host = this.getIntent().getStringExtra("HOST");
    if(host == null) {
      host = "http://10.0.2.2:8088/janus";
    }

    JanusConfImpl conf = new JanusConfImpl();
    conf.url(host);
    conf.plugin(JanusPlugins.STREAMING);

    JanusService service = new JanusService(this, this, conf);

    Streaming self = this;
    this._listAdapter = new BaseAdapter() {
      @Override
      public int getCount() {
        return self._descriptions.size();
      }

      @Override
      public Description getItem(int position) {
        return self._descriptions.get(position);
      }

      @Override
      public long getItemId(int position) {
        return self._descriptions.get(position).id;
      }

      @Override
      public View getView(int position, View convertView, ViewGroup parent) {
        if (convertView == null) {
          convertView = getLayoutInflater().inflate(R.layout.feed_item, parent, false);
        }
        Description item = this.getItem(position);

        TextView descriptionView = convertView.findViewById(R.id.feed_description);
        descriptionView.setText(Long.toString(item.id) + ": " + item.description);

        return convertView;
      }
    };
    list.setAdapter(this._listAdapter);
    list.setOnItemClickListener((parent, view, position, id) -> {
      Bundle payload = Bundle.create();

      payload.setInt("id", id);

      service.dispatch(JanusCommands.WATCH, payload);

      list.setEnabled(false);
    });

    service.statusListener().observe(this, status -> {
      if(status.equals("READY")) {
        Bundle context = Bundle.create();
        service.dispatch(JanusCommands.LIST, context);
      } else if(status.equals("OFF") || status.equals("CLOSED")) {
        this._start.setEnabled(true);
        this._stop.setEnabled(false);
        list.setEnabled(true);
        this._descriptions.clear();
        this._listAdapter.notifyDataSetChanged();
        this._streaming.setText("streaming: false");
        videoView.release();
      }

      statusView.setText(status);
    });

    this._start.setOnClickListener(v -> {
      this._stop.setEnabled(false);
      this._start.setEnabled(false);
      service.start();
    });

    this._stop.setOnClickListener(v ->{
      this._stop.setEnabled(false);
      this._start.setEnabled(false);
      service.stop();
    });

    service.remoteVideoTrackListener().observe(this, track -> {
      if(track == null) {
        return;
      }

      videoView.init(service.rootEgl().getEglBaseContext(), null);
      videoView.setScalingType(RendererCommon.ScalingType.SCALE_ASPECT_FILL);
      track.addSink(videoView);
    });

    service.remoteAudioTrackListener().observe(this, audioTrack -> {
      if(audioTrack == null) {
        return;
      }

      AudioManager audioManager = ((AudioManager) this.getSystemService(Context.AUDIO_SERVICE));
      audioManager.setMode(AudioManager.MODE_IN_COMMUNICATION);
      audioManager.setSpeakerphoneOn(true);
    });
  }

  @Override
  public void onEvent(JanusEvent event, Bundle payload) {
    JanusData data = event.data();

    if(data.getString("janus", "").equals("success") && payload.getString("command", "").equals(JanusCommands.LIST)) {
      this.runOnUiThread(() -> {
        ArrayList<JanusData> list = data.getObject("plugindata").getObject("data").getList("list");
        this._descriptions.clear();

        for(JanusData desc : list) {
          this._descriptions.add(new Description(desc));
        }

        this._start.setEnabled(false);
        this._stop.setEnabled(true);

        this._listAdapter.notifyDataSetChanged();
      });

      return;
    }

    if(data.getString("janus", "").equals("webrtcup")) {
      this.runOnUiThread(() -> {
        this._streaming.setText("streaming: true");
      });
    }
  }
}
