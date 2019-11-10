package com.github.helloiampau.janusclientsdk.rtc;

import org.webrtc.AudioTrack;
import org.webrtc.DataChannel;
import org.webrtc.VideoCapturer;
import org.webrtc.VideoTrack;

public class MutableMediaBundle extends MediaBundle {

  public void localVideoTrack(VideoTrack localTrack) {
    this._localVideoTrack = localTrack;
  }

  public void localAudioTrack(AudioTrack localTrack) {
    this._localAudioTrack = localTrack;
  }

  public void addRemoteVideoTrack(VideoTrack remoteVideoTrack) {
    this._remoteVideoTracks.add(remoteVideoTrack);
  }

  public void addRemoteAudioTrack(AudioTrack remoteAudioTrack) {
    this._remoteAudioTracks.add(remoteAudioTrack);
  }

  public void videoCapturer(VideoCapturer videoCapturer) {
    this._videoCapturer = videoCapturer;
  }

  public void dataChannel(DataChannel dataChannel) {
    this._dataChannel = dataChannel;
  }
}
