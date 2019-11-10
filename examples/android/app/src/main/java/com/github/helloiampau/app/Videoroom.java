package com.github.helloiampau.app;

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

import org.webrtc.SurfaceViewRenderer;

import java.util.ArrayList;
import java.util.List;

public class Videoroom extends AppCompatActivity implements ServiceDelegate {

  class Room {
    public long room;
    public String description;

    public Room(JanusData data) {
      this.room = data.getInt("room", -1);
      this.description = data.getString("description", "");
    }
  }

  private List<Room> _rooms = new ArrayList<>();
  private BaseAdapter _listAdapter;
  private Button _start;
  private Button _stop;
  private JanusService _service;
  private TextView _streaming;
  private Button _simulate;
  private long _selectedRoom;
  private ListView _list;

  private JanusService _simulatedService;

  @Override
  protected void onCreate(android.os.Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.videoroom);

    this._simulate = this.findViewById(R.id.videoroom_test_simulate_button);
    this._start = this.findViewById(R.id.videoroom_test_start_button);
    TextView statusView = this.findViewById(R.id.videoroom_test_status_text);
    this._stop = this.findViewById(R.id.videoroom_test_stop_button);
    this._list = this.findViewById(R.id.videoroom_test_feed_list);
    this._streaming = this.findViewById(R.id.videoroom_test_status);
    SurfaceViewRenderer localView = this.findViewById(R.id.videoroom_test_video);
    this._streaming.setText("streaming: false");

    String host = this.getIntent().getStringExtra("HOST");
    if(host == null) {
      host = "http://10.0.2.2:8088/janus";
    }

    JanusConfImpl conf = new JanusConfImpl();
    conf.url(host);
    conf.plugin(JanusPlugins.VIDEOROOM);

    this._service = new JanusService(this, this, conf);

    Videoroom self = this;
    this._listAdapter = new BaseAdapter() {
      @Override
      public int getCount() {
        return self._rooms.size();
      }

      @Override
      public Room getItem(int position) {
        return self._rooms.get(position);
      }

      @Override
      public long getItemId(int position) {
        return this.getItem(position).room;
      }

      @Override
      public View getView(int position, View convertView, ViewGroup parent) {
        if (convertView == null) {
          convertView = getLayoutInflater().inflate(R.layout.feed_item, parent, false);
        }
        Room item = this.getItem(position);

        TextView descriptionView = convertView.findViewById(R.id.feed_description);
        descriptionView.setText(Long.toString(item.room) + ": " + item.description);

        return convertView;
      }
    };
    this._list.setAdapter(this._listAdapter);
    this._list.setOnItemClickListener((parent, view, position, id) -> {
      this._selectedRoom = id;
      Bundle payload = Bundle.create();
      payload.setInt("room", id);
      this._service.dispatch(JanusCommands.JOIN, payload);
    });

    this._start.setOnClickListener(v -> {
      this._stop.setEnabled(false);
      this._simulate.setEnabled(false);
      this._start.setEnabled(false);
      this._service.start();
    });

    this._stop.setOnClickListener(v ->{
      this._stop.setEnabled(false);
      this._simulate.setEnabled(false);
      this._start.setEnabled(false);
      this._service.stop();

      if(this._simulatedService != null) {
        this._simulatedService.stop();
        this._simulatedService = null;
      }
    });

    this._service.statusListener().observe(this, status -> {
      if(status.equals("READY")) {
        Bundle context = Bundle.create();
        this._service.dispatch(JanusCommands.LIST, context);
      } else if(status.equals("OFF") || status.equals("CLOSED")) {
        this._start.setEnabled(true);
        this._stop.setEnabled(false);
        this._simulate.setEnabled(false);
        this._list.setEnabled(true);
        this._rooms.clear();
        this._listAdapter.notifyDataSetChanged();
        this._streaming.setText("streaming: false");
        localView.release();
      }

      statusView.setText(status);
    });

    this._simulate.setOnClickListener(l -> {
      // simulating a new publisher in room
      this._simulatedService = new JanusService(this, (JanusEvent event, Bundle context) -> {
        JanusData data = event.data();

        if(data.getString("videoroom", "").equals("joined")) {
          Bundle payload = Bundle.create();
          payload.setBool("audio", true);
          payload.setBool("video", true);
          payload.setBool("data", true);

          this._simulatedService.dispatch(JanusCommands.PUBLISH, payload);
        }
      }, conf);

      this._simulatedService.statusListener().observe(this, status -> {
        if(status.equals("READY")) {
          Bundle payload = Bundle.create();
          payload.setInt("room", this._selectedRoom);
          this._simulatedService.dispatch(JanusCommands.JOIN, payload);
        }
      });

      this._simulatedService.start();
    });
  }

  @Override
  public void onEvent(JanusEvent event, Bundle context) {
    JanusData data = event.data();

    if(data.getString("janus", "").equals("success") && context.getString("command", "").equals(JanusCommands.LIST)) {
      this.runOnUiThread(() -> {
        ArrayList<JanusData> list = data.getObject("plugindata").getObject("data").getList("list");
        this._rooms.clear();

        for(JanusData desc : list) {
          this._rooms.add(new Room(desc));
        }

        this._start.setEnabled(false);
        this._stop.setEnabled(true);

        this._listAdapter.notifyDataSetChanged();
      });

      return;
    }

    if(data.getString("videoroom", "").equals("joined")) {
      this.runOnUiThread(() -> {
        this._list.setEnabled(false);
        this._simulate.setEnabled(true);
      });

      return;
    }

    List<JanusData> publishers = data.getList("publishers");
    if(publishers.size() > 0) {
      for(JanusData publisher : publishers) {
        Bundle payload = Bundle.create();
        payload.setInt("room", this._selectedRoom);
        payload.setInt("feed", publisher.getInt("id", -1));

        this._service.dispatch(JanusCommands.SUBSCRIBE, payload);
      }

      return;
    }

    long unpublished = data.getInt("unpublished", -1);
    if(unpublished > -1) {
      return;
    }
  }

}
