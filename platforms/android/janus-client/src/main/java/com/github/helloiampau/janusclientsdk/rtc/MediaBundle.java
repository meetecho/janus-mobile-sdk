package com.github.helloiampau.janusclientsdk.rtc;

import org.webrtc.AudioTrack;
import org.webrtc.DataChannel;
import org.webrtc.VideoCapturer;
import org.webrtc.VideoTrack;

import java.util.ArrayList;
import java.util.List;

import static java.util.Collections.unmodifiableList;

public abstract class MediaBundle {

  protected VideoTrack _localVideoTrack;

  public VideoTrack localVideoTrack() {
    return this._localVideoTrack;
  }

  protected AudioTrack _localAudioTrack;

  public AudioTrack localAudioTrack() {
    return this._localAudioTrack;
  }

  protected List<VideoTrack> _remoteVideoTracks = new ArrayList<>();

  public List<VideoTrack> remoteVideoTracks() {
    return unmodifiableList(this._remoteVideoTracks);
  }

  protected List<AudioTrack> _remoteAudioTracks = new ArrayList<>();

  public List<AudioTrack> remoteAudioTracks() {
    return unmodifiableList(this._remoteAudioTracks);
  }

  protected VideoCapturer _videoCapturer;

  public VideoCapturer videoCapturer() {
    return this._videoCapturer;
  }

  protected DataChannel _dataChannel;

  public DataChannel dataChannel() {
    return this._dataChannel;
  }
}
