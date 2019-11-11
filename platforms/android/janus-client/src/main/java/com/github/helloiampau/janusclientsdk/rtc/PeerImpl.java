package com.github.helloiampau.janusclientsdk.rtc;

import android.content.Context;
import android.util.Log;

import com.github.helloiampau.janus.generated.Bundle;
import com.github.helloiampau.janus.generated.Camera;
import com.github.helloiampau.janus.generated.Constraints;
import com.github.helloiampau.janus.generated.JanusConf;
import com.github.helloiampau.janus.generated.Peer;
import com.github.helloiampau.janus.generated.Protocol;
import com.github.helloiampau.janus.generated.SdpConstraints;
import com.github.helloiampau.janus.generated.SdpType;
import com.github.helloiampau.janus.generated.VideoConstraints;
import com.github.helloiampau.janusclientsdk.JanusDelegate;

import org.webrtc.AudioSource;
import org.webrtc.AudioTrack;
import org.webrtc.CameraEnumerator;
import org.webrtc.DataChannel;
import org.webrtc.EglBase;
import org.webrtc.IceCandidate;
import org.webrtc.MediaConstraints;
import org.webrtc.MediaStream;
import org.webrtc.MediaStreamTrack;
import org.webrtc.PeerConnection;
import org.webrtc.PeerConnectionFactory;
import org.webrtc.RtpReceiver;
import org.webrtc.RtpTransceiver;
import org.webrtc.SdpObserver;
import org.webrtc.SessionDescription;
import org.webrtc.SurfaceTextureHelper;
import org.webrtc.VideoCapturer;
import org.webrtc.VideoSource;
import org.webrtc.VideoTrack;

import java.nio.ByteBuffer;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.function.Consumer;

public class PeerImpl extends Peer implements PeerConnection.Observer, DataChannel.Observer {

  public static String[] DEFAULT_ICE_SERVERS = {
    "stun:stun.l.google.com:19302",
    "stun:stun1.l.google.com:19302",
    "stun:stun2.l.google.com:19302",
    "stun:stun3.l.google.com:19302",
    "stun:stun4.l.google.com:19302"
  };

  private Context _appContext;
  private Protocol _owner;
  private JanusDelegate _delegate;
  private CameraEnumerator _cameraEnumerator;
  private PeerConnectionFactory _pcFactory;
  private EglBase _rootEglBase;

  private PeerConnection _peerConnection;
  private MutableMediaBundle _mediaBundle;

  private long _id;

  public PeerImpl(long id, JanusConf conf, Protocol owner, JanusDelegate delegate, Context appContext, PeerConnectionFactory pcFactory, CameraEnumerator cameraEnumerator, EglBase rootEglBase) {
    this._id = id;

    this._appContext = appContext;
    this._owner = owner;
    this._delegate = delegate;
    this._cameraEnumerator = cameraEnumerator;
    this._pcFactory = pcFactory;
    this._rootEglBase = rootEglBase;

    List<PeerConnection.IceServer> iceServers = new ArrayList<>();

    for (String server : PeerImpl.DEFAULT_ICE_SERVERS) {
      PeerConnection.IceServer iceServer = PeerConnection.IceServer.builder(server).createIceServer();
      iceServers.add(iceServer);
    }

    PeerConnection.RTCConfiguration rtcConf = new PeerConnection.RTCConfiguration(iceServers);
    rtcConf.sdpSemantics = PeerConnection.SdpSemantics.UNIFIED_PLAN;
    rtcConf.bundlePolicy = PeerConnection.BundlePolicy.MAXBUNDLE;

    this._peerConnection = pcFactory.createPeerConnection(rtcConf, this);

    this._mediaBundle = new MutableMediaBundle();
  }

  public MediaBundle getUserMedia(Constraints constraints) {
    SdpConstraints sdpConstraints = constraints.getSdp();

    if(sdpConstraints.getDatachannel() == true) {
      DataChannel.Init init = new DataChannel.Init();

      DataChannel dataChannel = this._peerConnection.createDataChannel("janus-client-datachannel", init);
      dataChannel.registerObserver(this);

      this._mediaBundle.dataChannel(dataChannel);
    }

    if (sdpConstraints.getSendVideo() == true) {
      VideoConstraints videoConstraints = constraints.getVideo();

      // creating video capturer
      boolean isFront = videoConstraints.getCamera() == Camera.FRONT;

      String[] devices = this._cameraEnumerator.getDeviceNames();
      String device = Arrays.stream(devices).filter(current -> {
        return this._cameraEnumerator.isFrontFacing(current) == isFront;
      }).findAny().get();
      VideoCapturer capturer = this._cameraEnumerator.createCapturer(device, null);
      this._mediaBundle.videoCapturer(capturer);

      // creating video source
      SurfaceTextureHelper captureThread = SurfaceTextureHelper.create("CaptureThread", this._rootEglBase.getEglBaseContext());
      VideoSource videoSource = this._pcFactory.createVideoSource(false);
      capturer.initialize(captureThread, this._appContext, videoSource.getCapturerObserver());
      capturer.startCapture(videoConstraints.getWidth(), videoConstraints.getHeight(), videoConstraints.getFps());

      // creating local video track
      VideoTrack videoTrack = this._pcFactory.createVideoTrack("janus-client-video", videoSource);
      videoTrack.setEnabled(true);

      // register local track
      this._peerConnection.addTrack(videoTrack);
      this._mediaBundle.localVideoTrack(videoTrack);
    }

    if (sdpConstraints.getSendAudio() == true) {
      // creating audio source
      MediaConstraints mediaConstraints = new MediaConstraints();
      AudioSource audioSource = this._pcFactory.createAudioSource(mediaConstraints);

      // creating local audio track
      AudioTrack audioTrack = this._pcFactory.createAudioTrack("janus-client-audio", audioSource);
      audioTrack.setEnabled(true);

      // register local track
      this._peerConnection.addTrack(audioTrack);
      this._mediaBundle.localAudioTrack(audioTrack);
    }

    this._delegate.onMediaChanged(this._mediaBundle);
    return this._mediaBundle;
  }

  @Override
  public void createOffer(Constraints constraints, Bundle context) {
    this.getUserMedia(constraints);

    this._createSdp(constraints, mediaConstraints -> {
      PeerImpl self = this;

      this._peerConnection.createOffer(new SdpObserver() {
        @Override
        public void onCreateSuccess(SessionDescription sessionDescription) {
          String sdp = self._delegate.onOffer(sessionDescription.description);

          self._owner.onOffer(sdp, context);
        }

        @Override
        public void onSetSuccess() {

        }

        @Override
        public void onCreateFailure(String s) {

        }

        @Override
        public void onSetFailure(String s) {

        }
      }, mediaConstraints);
    });
  }

  @Override
  public void createAnswer(Constraints constraints, Bundle context) {
    this.getUserMedia(constraints);

    this._createSdp(constraints, mediaConstraints -> {

      PeerImpl self = this;

      this._peerConnection.createAnswer(new SdpObserver() {
        @Override
        public void onCreateSuccess(SessionDescription sessionDescription) {
          String sdp = self._delegate.onAnswer(sessionDescription.description);

          self._owner.onAnswer(sdp, context);
        }

        @Override
        public void onSetSuccess() {

        }

        @Override
        public void onCreateFailure(String s) {

        }

        @Override
        public void onSetFailure(String s) {

        }
      }, mediaConstraints);
    });
  }

  @Override
  public void setLocalDescription(SdpType type, String sdp) {
    this._setSdp(type, sdp, description -> {
      this._peerConnection.setLocalDescription(new SdpObserver() {
        @Override
        public void onCreateSuccess(SessionDescription sessionDescription) {

        }

        @Override
        public void onSetSuccess() {
          Log.v("janus-client", "Local Set");
        }

        @Override
        public void onCreateFailure(String s) {

        }

        @Override
        public void onSetFailure(String s) {
          Log.v("janus-client", "Local Failed: " + s);
        }
      }, description);
    });
  }

  @Override
  public void setRemoteDescription(SdpType type, String sdp) {
    this._setSdp(type, sdp, description -> {
      this._peerConnection.setRemoteDescription(new SdpObserver() {
        @Override
        public void onCreateSuccess(SessionDescription sessionDescription) {}

        @Override
        public void onSetSuccess() {
          Log.v("janus-client", "Remote Set");
        }

        @Override
        public void onCreateFailure(String s) {}

        @Override
        public void onSetFailure(String s) {
          Log.v("janus-client", "Remote Failed: " + s);
        }
      }, description);
    });
  }

  @Override
  public void addIceCandidate(String mid, int index, String sdp) {
    IceCandidate candidate = new IceCandidate(mid, index, sdp);
    this._peerConnection.addIceCandidate(candidate);
  }

  @Override
  public void close() {
    DataChannel dataChannel = this._mediaBundle.dataChannel();

    if(dataChannel != null) {
      dataChannel.close();
    }

    this._peerConnection.close();

    try {
      VideoCapturer videoCapturer = this._mediaBundle.videoCapturer();

      if(videoCapturer == null) {
        return;
      }

      videoCapturer.stopCapture();
      videoCapturer.dispose();
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
  }

  @Override
  public void onIceCandidate(IceCandidate iceCandidate) {
    this._owner.onIceCandidate(iceCandidate.sdpMid, iceCandidate.sdpMLineIndex, iceCandidate.sdp, this._id);
  }

  @Override
  public void onIceGatheringChange(PeerConnection.IceGatheringState iceGatheringState) {
    if(iceGatheringState != PeerConnection.IceGatheringState.COMPLETE) {
      return;
    }

    this._owner.onIceCompleted(this._id);
  }

  @Override
  public void onTrack(RtpTransceiver transceiver) {
    MediaStreamTrack track = transceiver.getReceiver().track();

    if(track instanceof VideoTrack) {
      this._mediaBundle.addRemoteVideoTrack((VideoTrack) track);
    } else {
      this._mediaBundle.addRemoteAudioTrack((AudioTrack) track);
    }

    this._delegate.onMediaChanged(this._mediaBundle);
  }

  @Override
  public void onMessage(DataChannel.Buffer buffer) {
    this._delegate.onDatachannelMessage(buffer);
  }

  private void _createSdp(Constraints constraints, Consumer<MediaConstraints> kernel) {
    MediaConstraints mediaConstraints = new MediaConstraints();

    SdpConstraints sdpConstraints = constraints.getSdp();
    if (sdpConstraints.getReceiveAudio()) {
      mediaConstraints.mandatory.add(new MediaConstraints.KeyValuePair("OfferToReceiveAudio", "true"));
    }

    if (sdpConstraints.getReceiveVideo()) {
      mediaConstraints.mandatory.add(new MediaConstraints.KeyValuePair("OfferToReceiveVideo", "true"));
    }

    kernel.accept(mediaConstraints);
  }

  private void _setSdp(SdpType type, String sdp, Consumer<SessionDescription> kernel) {
    SessionDescription.Type sdpType = type.equals(SdpType.OFFER) ? SessionDescription.Type.OFFER : SessionDescription.Type.ANSWER;
    SessionDescription description = new SessionDescription(sdpType, sdp);

    kernel.accept(description);
  }

  @Override
  public void onSignalingChange(PeerConnection.SignalingState signalingState) {

  }

  @Override
  public void onConnectionChange(PeerConnection.PeerConnectionState newState) {

  }

  @Override
  public void onIceConnectionReceivingChange(boolean b) {

  }

  @Override
  public void onIceConnectionChange(PeerConnection.IceConnectionState iceConnectionState) {

  }

  @Override
  public void onIceCandidatesRemoved(IceCandidate[] iceCandidates) {

  }

  @Override
  public void onAddStream(MediaStream mediaStream) {
  }

  @Override
  public void onRemoveStream(MediaStream mediaStream) {
  }

  @Override
  public void onRenegotiationNeeded() {

  }

  @Override
  public void onAddTrack(RtpReceiver rtpReceiver, MediaStream[] mediaStreams) {
  }

  @Override
  public void onBufferedAmountChange(long l) {

  }

  @Override
  public void onStateChange() {

  }

  @Override
  public void onDataChannel(DataChannel dataChannel) {
  }
}
