package com.github.helloiampau.janusclientsdk.rtc;

import android.content.Context;

import com.github.helloiampau.janus.generated.JanusConf;
import com.github.helloiampau.janus.generated.Peer;
import com.github.helloiampau.janus.generated.PeerFactory;
import com.github.helloiampau.janus.generated.Protocol;
import com.github.helloiampau.janusclientsdk.JanusDelegate;

import org.webrtc.Camera1Enumerator;
import org.webrtc.Camera2Enumerator;
import org.webrtc.CameraEnumerator;
import org.webrtc.DefaultVideoDecoderFactory;
import org.webrtc.DefaultVideoEncoderFactory;
import org.webrtc.EglBase;
import org.webrtc.PeerConnectionFactory;
import org.webrtc.VideoDecoderFactory;
import org.webrtc.VideoEncoderFactory;
import org.webrtc.audio.AudioDeviceModule;
import org.webrtc.audio.JavaAudioDeviceModule;

public class PeerFactoryImpl extends PeerFactory {

  private JanusConf _conf;
  private JanusDelegate _delegate;
  private Context _appContext;
  private CameraEnumerator _cameraEnumerator;
  private EglBase _rootEglBase;

  public PeerFactoryImpl(JanusConf conf, JanusDelegate delegate, Context appContext, EglBase rootEglBase) {
    PeerConnectionFactory.InitializationOptions pcOptions = PeerConnectionFactory.InitializationOptions.builder(appContext).createInitializationOptions();
    PeerConnectionFactory.initialize(pcOptions);

    this._conf = conf;
    this._delegate = delegate;
    this._appContext = appContext;
    this._rootEglBase = rootEglBase;

    if(Camera2Enumerator.isSupported(this._appContext)) {
      this._cameraEnumerator = new Camera2Enumerator(appContext);
    } else {
      this._cameraEnumerator = new Camera1Enumerator(false);
    }
  }

  @Override
  public Peer create(long id, Protocol owner) {
    VideoDecoderFactory decoderFactory = new DefaultVideoDecoderFactory(this._rootEglBase.getEglBaseContext());
    VideoEncoderFactory encoderFactory = new DefaultVideoEncoderFactory(this._rootEglBase.getEglBaseContext(), true, true);
    AudioDeviceModule audioDeviceModule = JavaAudioDeviceModule.builder(this._appContext).createAudioDeviceModule();

    PeerConnectionFactory pcFactory = PeerConnectionFactory.builder()
                                          .setAudioDeviceModule(audioDeviceModule)
                                          .setVideoDecoderFactory(decoderFactory)
                                          .setVideoEncoderFactory(encoderFactory)
                                          .createPeerConnectionFactory();

    return new PeerImpl(id, this._conf, owner, this._delegate, this._appContext, pcFactory, this._cameraEnumerator, this._rootEglBase);
  }

}
