package com.github.helloiampau.janusclientsdk.rtc;

import org.junit.Test;
import org.webrtc.AudioTrack;
import org.webrtc.DataChannel;
import org.webrtc.VideoCapturer;
import org.webrtc.VideoTrack;

import java.util.ArrayList;
import java.util.List;

import static org.junit.Assert.*;
import static org.mockito.Mockito.mock;

public class MutableMediaBundleTest {

  @Test
  public void itShouldStoreAndRetrieveLocalVideoTrack() {
    VideoTrack videoTrack = mock(VideoTrack.class);

    MutableMediaBundle media = new MutableMediaBundle();
    media.localVideoTrack(videoTrack);

    assertEquals(videoTrack, media.localVideoTrack());
  }

  @Test
  public void itShouldStoreAndRetrieveLocalAudioTrack() {
    AudioTrack audioTrack = mock(AudioTrack.class);

    MutableMediaBundle media = new MutableMediaBundle();
    media.localAudioTrack(audioTrack);

    assertEquals(audioTrack, media.localAudioTrack());
  }

  @Test
  public void itShouldStoreAndRetrieveRemoteVideoTracks() {
    List<VideoTrack> videoTracks = new ArrayList<>();
    videoTracks.add(mock(VideoTrack.class));
    videoTracks.add(mock(VideoTrack.class));

    MutableMediaBundle media = new MutableMediaBundle();

    for(VideoTrack videoTrack : videoTracks) {
      media.addRemoteVideoTrack(videoTrack);
    }

    assertArrayEquals(media.remoteVideoTracks().toArray(), videoTracks.toArray());
  }

  @Test
  public void itShouldStoreAndRetrieveRemoteAudioTracks() {
    List<AudioTrack> audioTracks = new ArrayList<>();
    audioTracks.add(mock(AudioTrack.class));
    audioTracks.add(mock(AudioTrack.class));

    MutableMediaBundle media = new MutableMediaBundle();

    for(AudioTrack audioTrack : audioTracks) {
      media.addRemoteAudioTrack(audioTrack);
    }

    assertArrayEquals(media.remoteAudioTracks().toArray(), audioTracks.toArray());
  }

  @Test
  public void itShouldRegisterAVideoCapturer() {
    VideoCapturer capturer = mock(VideoCapturer.class);

    MutableMediaBundle media = new MutableMediaBundle();
    media.videoCapturer(capturer);

    assertEquals(capturer, media.videoCapturer());
  }

  @Test
  public void itShouldRegisterADatachannel() {
    DataChannel dataChannel = mock(DataChannel.class);

    MutableMediaBundle media = new MutableMediaBundle();
    media.dataChannel(dataChannel);

    assertEquals(dataChannel, media.dataChannel());
  }

}