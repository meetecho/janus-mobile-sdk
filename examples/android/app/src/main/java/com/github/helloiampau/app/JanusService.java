package com.github.helloiampau.app;

import android.Manifest;
import android.app.Activity;
import android.arch.lifecycle.LiveData;
import android.arch.lifecycle.MutableLiveData;

import com.github.helloiampau.app.custom.CustomPluginFactory;
import com.github.helloiampau.janus.generated.Bundle;
import com.github.helloiampau.janus.generated.Janus;
import com.github.helloiampau.janus.generated.JanusError;
import com.github.helloiampau.janus.generated.JanusEvent;
import com.github.helloiampau.janusclientsdk.JanusConfImpl;
import com.github.helloiampau.janusclientsdk.JanusDelegate;
import com.github.helloiampau.janusclientsdk.JanusFactory;
import com.github.helloiampau.janusclientsdk.rtc.MediaBundle;
import com.karumi.dexter.Dexter;
import com.karumi.dexter.MultiplePermissionsReport;
import com.karumi.dexter.PermissionToken;
import com.karumi.dexter.listener.PermissionRequest;
import com.karumi.dexter.listener.multi.MultiplePermissionsListener;

import org.webrtc.AudioTrack;
import org.webrtc.DataChannel;
import org.webrtc.EglBase;
import org.webrtc.VideoTrack;

import java.nio.ByteBuffer;
import java.nio.charset.Charset;
import java.util.List;

public class JanusService extends JanusDelegate {

  private Janus _janus;
  private EglBase _rootEgl;
  private ServiceDelegate _delegate;
  private MutableLiveData<String> _status = new MutableLiveData<>();
  private MutableLiveData<VideoTrack> _localVideoTrack = new MutableLiveData<>();
  private MutableLiveData<VideoTrack> _remoteVideoTrack = new MutableLiveData<>();
  private MutableLiveData<AudioTrack> _remoteAudioTrack = new MutableLiveData<>();
  private MutableLiveData<JanusError> _error = new MutableLiveData<>();
  private MutableLiveData<DataChannel> _datachannel = new MutableLiveData<>();
  private MutableLiveData<String> _log = new MutableLiveData<>();

  public JanusService(Activity activity, ServiceDelegate delegate, JanusConfImpl conf) {
    JanusService self = this;

    Dexter.withActivity(activity).withPermissions(Manifest.permission.CAMERA, Manifest.permission.RECORD_AUDIO).withListener(new MultiplePermissionsListener() {
      @Override
      public void onPermissionsChecked(MultiplePermissionsReport report) {
        self._status.postValue("OFF");

        JanusFactory factory = new JanusFactory();

        // adding an useless plugin
        CustomPluginFactory pluginFactory = new CustomPluginFactory();
        factory.pluginFactory("my.yolo.plugin", pluginFactory);

        self._janus = factory.create(conf, self, activity.getApplicationContext());
        self._rootEgl = factory.rootEgl();

        self._delegate = delegate;
      }

      @Override
      public void onPermissionRationaleShouldBeShown(List<PermissionRequest> permissions, PermissionToken token) {

      }
    }).check();
  }

  public LiveData<String> statusListener() {
    return this._status;
  }

  public LiveData<VideoTrack> localVideoTrackListener() {
    return this._localVideoTrack;
  }

  public LiveData<VideoTrack> remoteVideoTrackListener() {
    return this._remoteVideoTrack;
  }

  public LiveData<AudioTrack> remoteAudioTrackListener() {
    return this._remoteAudioTrack;
  }

  public LiveData<DataChannel> dataChannelListener() { return this._datachannel; }

  public LiveData<String> logListener() {
    return this._log;
  }

  public LiveData<JanusError> errorListener() {
    return this._error;
  }

  public void start() {
    this._janus.init();
  }

  public void stop() {
    this._janus.close();
  }

  public void hangup() {
    this._janus.hangup();
  }

  public EglBase rootEgl() {
    return this._rootEgl;
  }

  public void dispatch(String command, Bundle payload) {
    this._janus.dispatch(command, payload);
  }

  @Override
  public void onClose() {
    this._status.postValue("CLOSED");
  }

  @Override
  public void onError(JanusError error, Bundle context) {
    this._error.postValue(error);
  }

  @Override
  public void onReady() {
    this._status.postValue("READY");
  }

  @Override
  public void onHangup(String reason) {
    this._status.postValue("READY");
  }

  @Override
  public void onMediaChanged(MediaBundle media) {
    VideoTrack currentLocal = this._localVideoTrack.getValue();
    VideoTrack receivedLocal = media.localVideoTrack();
    if(currentLocal != receivedLocal) {
      this._localVideoTrack.postValue(receivedLocal);
    }

    DataChannel currentDataChannel = this._datachannel.getValue();
    DataChannel receivedDataChannel = media.dataChannel();
    if(currentDataChannel != receivedDataChannel) {
      this._datachannel.postValue(receivedDataChannel);
    }

    if(media.remoteVideoTracks().size() == 1) {
      VideoTrack currentRemote = this._remoteVideoTrack.getValue();
      VideoTrack receivedRemote = media.remoteVideoTracks().get(0);

      if(currentRemote != receivedRemote) {
        this._remoteVideoTrack.postValue(receivedRemote);
      }
    }

    if(media.remoteAudioTracks().size() == 1) {
      AudioTrack currentRemote = this._remoteAudioTrack.getValue();
      AudioTrack receivedRemote = media.remoteAudioTracks().get(0);

      if(currentRemote != receivedRemote) {
        this._remoteAudioTrack.postValue(receivedRemote);
      }
    }
  }

  @Override
  public void onDatachannelMessage(DataChannel.Buffer buffer) {
    String current = this._log.getValue();

    if(current == null) {
      current = "";
    }

    ByteBuffer data = buffer.data;
    final byte[] bytes = new byte[data.capacity()];
    data.get(bytes);
    String strData = new String(bytes, Charset.forName("UTF-8"));

    current = current.concat(strData + "\n");
    this._log.postValue(current);
  }

  @Override
  public String onOffer(String sdp) {
    return sdp;
  }

  @Override
  public String onAnswer(String sdp) {
    return sdp;
  }

  @Override
  public void onEvent(JanusEvent event, Bundle context) {
    this._delegate.onEvent(event, context);
  }
}
