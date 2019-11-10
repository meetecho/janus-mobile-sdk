package com.github.helloiampau.janusclientsdk.rtc;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;

import com.github.helloiampau.janus.generated.Bundle;
import com.github.helloiampau.janus.generated.Camera;
import com.github.helloiampau.janus.generated.Constraints;
import com.github.helloiampau.janus.generated.JanusConf;
import com.github.helloiampau.janus.generated.Protocol;
import com.github.helloiampau.janus.generated.SdpConstraints;
import com.github.helloiampau.janus.generated.SdpType;
import com.github.helloiampau.janus.generated.VideoConstraints;
import com.github.helloiampau.janusclientsdk.JanusDelegate;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.webrtc.AudioSource;
import org.webrtc.AudioTrack;
import org.webrtc.CameraEnumerator;
import org.webrtc.CameraVideoCapturer;
import org.webrtc.CapturerObserver;
import org.webrtc.DataChannel;
import org.webrtc.EglBase;
import org.webrtc.IceCandidate;
import org.webrtc.MediaConstraints;
import org.webrtc.MediaStreamTrack;
import org.webrtc.PeerConnection;
import org.webrtc.PeerConnectionFactory;
import org.webrtc.RtpReceiver;
import org.webrtc.RtpTransceiver;
import org.webrtc.SdpObserver;
import org.webrtc.SessionDescription;
import org.webrtc.SurfaceTextureHelper;
import org.webrtc.VideoSource;
import org.webrtc.VideoTrack;

import java.util.List;

import static org.junit.Assert.*;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.ArgumentMatchers.isNull;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;
import static org.powermock.api.mockito.PowerMockito.mockStatic;
import static org.powermock.api.mockito.PowerMockito.whenNew;

@RunWith(PowerMockRunner.class)
@PrepareForTest({ SurfaceTextureHelper.class, PeerImpl.class })
public class PeerImplTest {

  private JanusConf _conf;
  private JanusDelegate _delegate;
  private PeerConnectionFactory _pcFactory;
  private PeerConnection _peerConnection;
  private Context _appContext;

  private VideoConstraints _video;
  private SdpConstraints _sdp;
  private Constraints _constraints;
  private CameraEnumerator _cameraEnumerator;
  private CameraVideoCapturer _cameraCapturer;
  private EglBase _eglBase;
  private EglBase.Context _eglContext;
  private SurfaceTextureHelper _captureThread;
  private VideoSource _videoSource;
  private CapturerObserver _capturerObserver;
  private VideoTrack _videoTrack;
  private AudioTrack _audioTrack;
  private AudioSource _audioSource;
  private Protocol _owner;
  private SessionDescription _offer;
  private SessionDescription _answer;
  private MutableMediaBundle _mediaBundle;
  private DataChannel _dataChannel;
  private DataChannel.Init _dataChannelInit;

  @Before
  public void beforeEach() throws Exception {
    mockStatic(SurfaceTextureHelper.class);

    this._capturerObserver = mock(CapturerObserver.class);

    this._videoSource = mock(VideoSource.class);
    when(this._videoSource.getCapturerObserver()).thenReturn(this._capturerObserver);

    this._audioSource = mock(AudioSource.class);

    this._videoTrack = mock(VideoTrack.class);

    this._audioTrack = mock(AudioTrack.class);

    this._conf = mock(JanusConf.class);
    this._delegate = mock(JanusDelegate.class);

    this._dataChannel = mock(DataChannel.class);

    this._dataChannelInit = mock(DataChannel.Init.class);
    whenNew(DataChannel.Init.class).withNoArguments().thenReturn(this._dataChannelInit);

    this._peerConnection = mock(PeerConnection.class);

    when(this._peerConnection.createDataChannel("janus-client-datachannel", this._dataChannelInit)).thenReturn(this._dataChannel);

    this._pcFactory = mock(PeerConnectionFactory.class);
    when(this._pcFactory.createVideoSource(false)).thenReturn(this._videoSource);
    when(this._pcFactory.createVideoTrack("janus-client-video", this._videoSource)).thenReturn(this._videoTrack);

    when(this._pcFactory.createAudioSource(any(MediaConstraints.class))).thenReturn(this._audioSource);
    when(this._pcFactory.createAudioTrack("janus-client-audio", this._audioSource)).thenReturn(this._audioTrack);

    when(this._pcFactory.createPeerConnection(any(PeerConnection.RTCConfiguration.class), any(PeerConnection.Observer.class))).thenReturn(this._peerConnection);

    this._appContext = mock(Context.class);
    when(this._appContext.checkSelfPermission(Manifest.permission.CAMERA)).thenReturn(PackageManager.PERMISSION_GRANTED);
    when(this._appContext.checkSelfPermission(Manifest.permission.RECORD_AUDIO)).thenReturn(PackageManager.PERMISSION_GRANTED);

    this._video = mock(VideoConstraints.class);
    when(this._video.getCamera()).thenReturn(Camera.FRONT);
    when(this._video.getFps()).thenReturn(30);
    when(this._video.getHeight()).thenReturn(720);
    when(this._video.getWidth()).thenReturn(1280);

    this._sdp = mock(SdpConstraints.class);
    when(this._sdp.getDatachannel()).thenReturn(false);
    when(this._sdp.getSendVideo()).thenReturn(false);
    when(this._sdp.getSendAudio()).thenReturn(false);
    when(this._sdp.getReceiveVideo()).thenReturn(false);
    when(this._sdp.getReceiveAudio()).thenReturn(false);

    this._constraints = mock(Constraints.class);
    when(this._constraints.getVideo()).thenReturn(this._video);
    when(this._constraints.getSdp()).thenReturn(this._sdp);

    this._cameraCapturer = mock(CameraVideoCapturer.class);

    this._cameraEnumerator = mock(CameraEnumerator.class);
    String[] devices = { "front_camera", "back_camera" };
    when(this._cameraEnumerator.getDeviceNames()).thenReturn(devices);
    when(this._cameraEnumerator.isFrontFacing("front_camera")).thenReturn(true);
    when(this._cameraEnumerator.isFrontFacing("back_camera")).thenReturn(false);
    when(this._cameraEnumerator.createCapturer(anyString(), isNull())).thenReturn(this._cameraCapturer);

    this._eglContext = mock(EglBase.Context.class);

    this._eglBase = mock(EglBase.class);
    when(this._eglBase.getEglBaseContext()).thenReturn(this._eglContext);

    this._captureThread = mock(SurfaceTextureHelper.class);
    when(SurfaceTextureHelper.create("CaptureThread", this._eglContext)).thenReturn(this._captureThread);

    this._owner = mock(Protocol.class);

    this._offer = new SessionDescription(SessionDescription.Type.OFFER, "the offer");
    this._answer = new SessionDescription(SessionDescription.Type.ANSWER, "the answer");

    when(this._delegate.onOffer(this._offer.description)).thenReturn(this._offer.description);
    when(this._delegate.onAnswer(this._answer.description)).thenReturn(this._answer.description);

    this._mediaBundle = mock(MutableMediaBundle.class);
    whenNew(MutableMediaBundle.class).withNoArguments().thenReturn(this._mediaBundle);
  }

  @Test
  public void itShouldCreateAnInnerWebRTCPeerconnection() {
    PeerImpl pc = new PeerImpl(-1, this._conf, this._owner, this._delegate, this._appContext, this._pcFactory, this._cameraEnumerator, this._eglBase);

    ArgumentCaptor<PeerConnection.RTCConfiguration> confCaptor = ArgumentCaptor.forClass(PeerConnection.RTCConfiguration.class);
    verify(this._pcFactory).createPeerConnection(confCaptor.capture(), eq(pc));

    List<PeerConnection.IceServer> defaultIceServers = confCaptor.getValue().iceServers;
    for(int index = 0; index < PeerImpl.DEFAULT_ICE_SERVERS.length; index++) {
      assertEquals(PeerImpl.DEFAULT_ICE_SERVERS[index], defaultIceServers.get(index).urls.get(0));
    }
    assertEquals(PeerConnection.SdpSemantics.UNIFIED_PLAN, confCaptor.getValue().sdpSemantics);
    assertEquals(PeerConnection.BundlePolicy.MAXBUNDLE, confCaptor.getValue().bundlePolicy);
  }

  @Test
  public void itShouldGetTheUserVideo() {
    when(this._sdp.getSendVideo()).thenReturn(true);

    PeerImpl pc = new PeerImpl(-1, this._conf, this._owner, this._delegate, this._appContext, this._pcFactory, this._cameraEnumerator, this._eglBase);

    MediaBundle userMedia = pc.getUserMedia(this._constraints);

    verify(this._cameraEnumerator).createCapturer("front_camera", null);
    verify(this._cameraCapturer).initialize(this._captureThread, this._appContext, this._capturerObserver);
    verify(this._cameraCapturer).startCapture(this._video.getWidth(), this._video.getHeight(), this._video.getFps());

    verify(this._pcFactory).createVideoTrack("janus-client-video", this._videoSource);
    verify(this._peerConnection).addTrack(this._videoTrack);

    verify(this._videoTrack).setEnabled(true);

    verify(this._mediaBundle).localVideoTrack(this._videoTrack);
  }

  @Test
  public void itShouldGetTheUserAudio() {
    when(this._sdp.getSendAudio()).thenReturn(true);

    PeerImpl pc = new PeerImpl(-1, this._conf, this._owner, this._delegate, this._appContext, this._pcFactory, this._cameraEnumerator, this._eglBase);

    MediaBundle userMedia = pc.getUserMedia(this._constraints);

    verify(this._pcFactory).createAudioTrack("janus-client-audio", this._audioSource);
    verify(this._peerConnection).addTrack(this._audioTrack);

    verify(this._audioTrack).setEnabled(true);
    verify(this._mediaBundle).localAudioTrack(this._audioTrack);
  }

  @Test
  public void itShouldCreateADatachannel() {
    when(this._sdp.getDatachannel()).thenReturn(true);

    PeerImpl pc = new PeerImpl(-1, this._conf, this._owner, this._delegate, this._appContext, this._pcFactory, this._cameraEnumerator, this._eglBase);

    MediaBundle userMedia = pc.getUserMedia(this._constraints);
    verify(this._mediaBundle).dataChannel(this._dataChannel);
    verify(this._dataChannel).registerObserver(pc);
  }

  @Test
  public void itShouldSkipTheDatachannelIfDatachannelIsFalse() {
    PeerImpl pc = new PeerImpl(-1, this._conf, this._owner, this._delegate, this._appContext, this._pcFactory, this._cameraEnumerator, this._eglBase);

    MediaBundle userMedia = pc.getUserMedia(this._constraints);

    verify(this._peerConnection, times(0)).createDataChannel(any(), any());
  }

  @Test
  public void itShouldSkipTheUserVideoIfSendVideoIsFalse() {
    PeerImpl pc = new PeerImpl(-1, this._conf, this._owner, this._delegate, this._appContext, this._pcFactory, this._cameraEnumerator, this._eglBase);

    MediaBundle userMedia = pc.getUserMedia(this._constraints);

    verify(this._cameraEnumerator, times(0)).createCapturer(any(), any());
    verify(this._cameraCapturer, times(0)).initialize(any(), any(), any());
    verify(this._cameraCapturer, times(0)).startCapture(this._video.getWidth(), this._video.getHeight(), this._video.getFps());

    verify(this._pcFactory, times(0)).createVideoTrack(any(), any());
    verify(this._peerConnection, times(0)).addTrack(any());

    verify(this._videoTrack, times(0)).setEnabled(true);

    assertEquals(userMedia.localVideoTrack(), null);
  }

  @Test
  public void itShouldSkipTheUserAudioIfSendAudioIsFalse() {
    PeerImpl pc = new PeerImpl(-1, this._conf, this._owner, this._delegate, this._appContext, this._pcFactory, this._cameraEnumerator, this._eglBase);

    MediaBundle userMedia = pc.getUserMedia(this._constraints);

    verify(this._pcFactory, times(0)).createAudioTrack(any(), any());
    verify(this._peerConnection, times(0)).addTrack(any());

    verify(this._audioTrack, times(0)).setEnabled(true);

    assertEquals(userMedia.localAudioTrack(), null);
  }

  @Test
  public void itShouldCreateAnOffer() {
    PeerImpl pc = new PeerImpl(-1, this._conf, this._owner, this._delegate, this._appContext, this._pcFactory, this._cameraEnumerator, this._eglBase);

    when(this._sdp.getReceiveAudio()).thenReturn(true);
    when(this._sdp.getReceiveVideo()).thenReturn(true);

    Bundle context = mock(Bundle.class);

    pc.createOffer(this._constraints, context);

    ArgumentCaptor<MediaConstraints> mediaConstraintsArgumentCaptor = ArgumentCaptor.forClass(MediaConstraints.class);
    ArgumentCaptor<SdpObserver> observerArgumentCaptor = ArgumentCaptor.forClass(SdpObserver.class);

    verify(this._peerConnection).createOffer(observerArgumentCaptor.capture(), mediaConstraintsArgumentCaptor.capture());

    MediaConstraints mediaConstraints = mediaConstraintsArgumentCaptor.getValue();
    assertEquals("OfferToReceiveAudio", mediaConstraints.mandatory.get(0).getKey());
    assertEquals("true", mediaConstraints.mandatory.get(0).getValue());

    assertEquals("OfferToReceiveVideo", mediaConstraints.mandatory.get(1).getKey());
    assertEquals("true", mediaConstraints.mandatory.get(1).getValue());

    observerArgumentCaptor.getValue().onCreateSuccess(this._offer);

    verify(this._delegate).onOffer(this._offer.description);
    verify(this._owner).onOffer("the offer", context);
    verify(this._delegate).onMediaChanged(this._mediaBundle);
  }

  @Test
  public void itShouldSkipMediaConstraintsForSendOnlyOffer() {
    PeerImpl pc = new PeerImpl(-1, this._conf, this._owner, this._delegate, this._appContext, this._pcFactory, this._cameraEnumerator, this._eglBase);

    Bundle context = mock(Bundle.class);

    pc.createOffer(this._constraints, context);

    ArgumentCaptor<MediaConstraints> mediaConstraintsArgumentCaptor = ArgumentCaptor.forClass(MediaConstraints.class);
    verify(this._peerConnection).createOffer(any(SdpObserver.class), mediaConstraintsArgumentCaptor.capture());

    MediaConstraints mediaConstraints = mediaConstraintsArgumentCaptor.getValue();
    assertEquals(0, mediaConstraints.mandatory.size());
  }

  @Test
  public void itShouldCreateAnAnswer() {
    PeerImpl pc = new PeerImpl(-1, this._conf, this._owner, this._delegate, this._appContext, this._pcFactory, this._cameraEnumerator, this._eglBase);

    when(this._sdp.getReceiveAudio()).thenReturn(true);
    when(this._sdp.getReceiveVideo()).thenReturn(true);

    Bundle context = mock(Bundle.class);

    pc.createAnswer(this._constraints, context);

    ArgumentCaptor<MediaConstraints> mediaConstraintsArgumentCaptor = ArgumentCaptor.forClass(MediaConstraints.class);
    ArgumentCaptor<SdpObserver> observerArgumentCaptor = ArgumentCaptor.forClass(SdpObserver.class);

    verify(this._peerConnection).createAnswer(observerArgumentCaptor.capture(), mediaConstraintsArgumentCaptor.capture());

    MediaConstraints mediaConstraints = mediaConstraintsArgumentCaptor.getValue();
    assertEquals("OfferToReceiveAudio", mediaConstraints.mandatory.get(0).getKey());
    assertEquals("true", mediaConstraints.mandatory.get(0).getValue());

    assertEquals("OfferToReceiveVideo", mediaConstraints.mandatory.get(1).getKey());
    assertEquals("true", mediaConstraints.mandatory.get(1).getValue());

    observerArgumentCaptor.getValue().onCreateSuccess(this._answer);

    verify(this._delegate).onAnswer(this._answer.description);
    verify(this._owner).onAnswer("the answer", context);
  }

  @Test
  public void itShouldSkipMediaConstraintsForSendOnlyAnswer() {
    PeerImpl pc = new PeerImpl(-1, this._conf, this._owner, this._delegate, this._appContext, this._pcFactory, this._cameraEnumerator, this._eglBase);

    Bundle context = mock(Bundle.class);

    pc.createAnswer(this._constraints, context);

    ArgumentCaptor<MediaConstraints> mediaConstraintsArgumentCaptor = ArgumentCaptor.forClass(MediaConstraints.class);
    verify(this._peerConnection).createAnswer(any(SdpObserver.class), mediaConstraintsArgumentCaptor.capture());

    MediaConstraints mediaConstraints = mediaConstraintsArgumentCaptor.getValue();
    assertEquals(0, mediaConstraints.mandatory.size());
  }

  @Test
  public void itShouldSetTheLocalDescription() {
    PeerImpl pc = new PeerImpl(-1, this._conf, this._owner, this._delegate, this._appContext, this._pcFactory, this._cameraEnumerator, this._eglBase);

    pc.setLocalDescription(SdpType.OFFER, "the offer sdp");
    pc.setLocalDescription(SdpType.ANSWER, "the answer sdp");

    ArgumentCaptor<SessionDescription> sdpCaptor = ArgumentCaptor.forClass(SessionDescription.class);
    verify(this._peerConnection, times(2)).setLocalDescription(any(SdpObserver.class), sdpCaptor.capture());

    assertEquals(sdpCaptor.getAllValues().get(0).type, SessionDescription.Type.OFFER);
    assertEquals(sdpCaptor.getAllValues().get(0).description, "the offer sdp");

    assertEquals(sdpCaptor.getAllValues().get(1).type, SessionDescription.Type.ANSWER);
    assertEquals(sdpCaptor.getAllValues().get(1).description, "the answer sdp");
  }

  @Test
  public void itShouldSetTheRemoteDescription() {
    PeerImpl pc = new PeerImpl(-1, this._conf, this._owner, this._delegate, this._appContext, this._pcFactory, this._cameraEnumerator, this._eglBase);

    pc.setRemoteDescription(SdpType.OFFER, "the offer sdp");
    pc.setRemoteDescription(SdpType.ANSWER, "the answer sdp");

    ArgumentCaptor<SessionDescription> sdpCaptor = ArgumentCaptor.forClass(SessionDescription.class);
    verify(this._peerConnection, times(2)).setRemoteDescription(any(SdpObserver.class), sdpCaptor.capture());

    assertEquals(sdpCaptor.getAllValues().get(0).type, SessionDescription.Type.OFFER);
    assertEquals(sdpCaptor.getAllValues().get(0).description, "the offer sdp");

    assertEquals(sdpCaptor.getAllValues().get(1).type, SessionDescription.Type.ANSWER);
    assertEquals(sdpCaptor.getAllValues().get(1).description, "the answer sdp");
  }

  @Test
  public void itShouldAddAnIceCandidate() {
    PeerImpl pc = new PeerImpl(-1, this._conf, this._owner, this._delegate, this._appContext, this._pcFactory, this._cameraEnumerator, this._eglBase);

    pc.addIceCandidate("yolo", 420, "the line");

    ArgumentCaptor<IceCandidate> candidateCaptor = ArgumentCaptor.forClass(IceCandidate.class);
    verify(this._peerConnection).addIceCandidate(candidateCaptor.capture());

    IceCandidate candidate = candidateCaptor.getValue();
    assertEquals(candidate.sdpMid, "yolo");
    assertEquals(candidate.sdpMLineIndex, 420);
    assertEquals(candidate.sdp, "the line");
  }

  @Test
  public void itShouldDelegateToPluginTheOnIceCandidateEvent() {
    PeerImpl pc = new PeerImpl(-1, this._conf, this._owner, this._delegate, this._appContext, this._pcFactory, this._cameraEnumerator, this._eglBase);

    IceCandidate candidate = new IceCandidate("yolo", 420, "the line");
    pc.onIceCandidate(candidate);

    verify(this._owner).onIceCandidate("yolo", 420, "the line");
  }

  @Test
  public void itShouldDelegateTheIceCompletedEvent() {
    PeerImpl pc = new PeerImpl(-1, this._conf, this._owner, this._delegate, this._appContext, this._pcFactory, this._cameraEnumerator, this._eglBase);
    pc.onIceGatheringChange(PeerConnection.IceGatheringState.COMPLETE);

    verify(this._owner, times(1)).onIceCompleted();
  }

  @Test
  public void itShouldSkipOtherIceEvents() {
    PeerImpl pc = new PeerImpl(-1, this._conf, this._owner, this._delegate, this._appContext, this._pcFactory, this._cameraEnumerator, this._eglBase);
    pc.onIceGatheringChange(PeerConnection.IceGatheringState.NEW);
    pc.onIceGatheringChange(PeerConnection.IceGatheringState.GATHERING);

    verify(this._owner, times(0)).onIceCompleted();
  }


  @Test
  public void itShouldUpdateTheMediaBundleWithARemoteVideoTrack() {
    MediaStreamTrack track = mock(VideoTrack.class);

    RtpReceiver receiver = mock(RtpReceiver.class);
    when(receiver.track()).thenReturn(track);

    RtpTransceiver transceiver = mock(RtpTransceiver.class);
    when(transceiver.getReceiver()).thenReturn(receiver);

    PeerImpl pc = new PeerImpl(-1, this._conf, this._owner, this._delegate, this._appContext, this._pcFactory, this._cameraEnumerator, this._eglBase);
    pc.onTrack(transceiver);

    verify(this._mediaBundle).addRemoteVideoTrack((VideoTrack) track);
    verify(this._delegate).onMediaChanged(this._mediaBundle);
  }

  @Test
  public void itShouldUpdateTheMediaBundleWithARemoteAudioTrack() {
    MediaStreamTrack track = mock(AudioTrack.class);

    RtpReceiver receiver = mock(RtpReceiver.class);
    when(receiver.track()).thenReturn(track);

    RtpTransceiver transceiver = mock(RtpTransceiver.class);
    when(transceiver.getReceiver()).thenReturn(receiver);

    PeerImpl pc = new PeerImpl(-1, this._conf, this._owner, this._delegate, this._appContext, this._pcFactory, this._cameraEnumerator, this._eglBase);
    pc.onTrack(transceiver);

    verify(this._mediaBundle).addRemoteAudioTrack((AudioTrack) track);
    verify(this._delegate).onMediaChanged(this._mediaBundle);
  }

  @Test
  public void itShouldCloseTheInnerPeerConnectionOnClose() {
    PeerImpl pc = new PeerImpl(-1, this._conf, this._owner, this._delegate, this._appContext, this._pcFactory, this._cameraEnumerator, this._eglBase);
    pc.close();

    verify(this._peerConnection).close();
  }

  @Test
  public void itShouldCloseTheVideoCapturerOnClose() throws InterruptedException {
    when(this._mediaBundle.videoCapturer()).thenReturn(this._cameraCapturer);

    PeerImpl pc = new PeerImpl(-1, this._conf, this._owner, this._delegate, this._appContext, this._pcFactory, this._cameraEnumerator, this._eglBase);
    pc.close();

    verify(this._cameraCapturer).stopCapture();
    verify(this._cameraCapturer).dispose();
  }

  @Test
  public void itShouldCloseTheDatachannelOnClose() throws InterruptedException {
    when(this._mediaBundle.dataChannel()).thenReturn(this._dataChannel);

    PeerImpl pc = new PeerImpl(-1, this._conf, this._owner, this._delegate, this._appContext, this._pcFactory, this._cameraEnumerator, this._eglBase);
    pc.close();

    verify(this._dataChannel).close();
  }

  @Test
  public void itShouldDelegateTheOnMessageDatachannelEvent() {
    DataChannel.Buffer buffer = mock(DataChannel.Buffer.class);

    PeerImpl pc = new PeerImpl(-1, this._conf, this._owner, this._delegate, this._appContext, this._pcFactory, this._cameraEnumerator, this._eglBase);
    pc.onMessage(buffer);

    verify(this._delegate).onDatachannelMessage(buffer);
  }

}