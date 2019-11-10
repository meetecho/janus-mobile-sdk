package com.github.helloiampau.janusclientsdk.rtc;

import android.content.Context;

import com.github.helloiampau.janus.generated.JanusConf;
import com.github.helloiampau.janus.generated.Protocol;
import com.github.helloiampau.janusclientsdk.JanusDelegate;


import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.webrtc.Camera1Enumerator;
import org.webrtc.Camera2Enumerator;
import org.webrtc.DefaultVideoDecoderFactory;
import org.webrtc.DefaultVideoEncoderFactory;
import org.webrtc.EglBase;
import org.webrtc.PeerConnectionFactory;
import org.webrtc.audio.JavaAudioDeviceModule;

import static org.junit.Assert.*;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;
import static org.powermock.api.mockito.PowerMockito.mockStatic;
import static org.powermock.api.mockito.PowerMockito.verifyNew;
import static org.powermock.api.mockito.PowerMockito.verifyStatic;
import static org.powermock.api.mockito.PowerMockito.whenNew;

@RunWith(PowerMockRunner.class)
@PrepareForTest({ JavaAudioDeviceModule.class, PeerConnectionFactory.class, PeerConnectionFactory.InitializationOptions.class, Camera2Enumerator.class, Camera1Enumerator.class, PeerFactoryImpl.class })
public class PeerFactoryImplTest {

  private PeerConnectionFactory.InitializationOptions.Builder _confBuilder;
  private PeerConnectionFactory.Builder _pcFactoryBuilder;
  private Context _appContext;
  private PeerConnectionFactory.InitializationOptions _options;
  private JanusConf _conf;
  private JanusDelegate _delegate;
  private PeerConnectionFactory _pcFactory;
  private EglBase _eglBase;
  private EglBase.Context _eglBaseContext;
  private DefaultVideoEncoderFactory _encoderFactory;
  private DefaultVideoDecoderFactory _decoderFactory;
  private JavaAudioDeviceModule _audioDeviceModule;
  private JavaAudioDeviceModule.Builder _audioBuilder;

  @Before
  public void beforeEach() throws Exception {
    mockStatic(PeerConnectionFactory.class);
    mockStatic(PeerConnectionFactory.InitializationOptions.class);
    mockStatic(Camera2Enumerator.class);
    mockStatic(Camera1Enumerator.class);
    mockStatic(JavaAudioDeviceModule.class);

    this._pcFactory = mock(PeerConnectionFactory.class);

    this._confBuilder = mock(PeerConnectionFactory.InitializationOptions.Builder.class);
    this._options = mock(PeerConnectionFactory.InitializationOptions.class);
    when(this._confBuilder.createInitializationOptions()).thenReturn(this._options);

    this._appContext = mock(Context.class);
    when(PeerConnectionFactory.InitializationOptions.builder(this._appContext)).thenReturn(this._confBuilder);

    this._conf = mock(JanusConf.class);
    this._delegate = mock(JanusDelegate.class);

    when(Camera2Enumerator.isSupported(this._appContext)).thenReturn(true);

    Camera1Enumerator camera1Enumerator = mock(Camera1Enumerator.class);
    whenNew(Camera1Enumerator.class).withArguments(false).thenReturn(camera1Enumerator);

    Camera2Enumerator camera2Enumerator = mock(Camera2Enumerator.class);
    whenNew(Camera2Enumerator.class).withArguments(this._appContext).thenReturn(camera2Enumerator);

    this._eglBaseContext = mock(EglBase.Context.class);
    this._eglBase = mock(EglBase.class);
    when(this._eglBase.getEglBaseContext()).thenReturn(this._eglBaseContext);

    this._decoderFactory = mock(DefaultVideoDecoderFactory.class);
    whenNew(DefaultVideoDecoderFactory.class).withArguments(this._eglBaseContext).thenReturn(this._decoderFactory);

    this._encoderFactory = mock(DefaultVideoEncoderFactory.class);
    whenNew(DefaultVideoEncoderFactory.class).withArguments(this._eglBaseContext, true, true).thenReturn(this._encoderFactory);

    this._audioDeviceModule = mock(JavaAudioDeviceModule.class);

    this._audioBuilder = mock(JavaAudioDeviceModule.Builder.class);
    when(this._audioBuilder.createAudioDeviceModule()).thenReturn(this._audioDeviceModule);

    when(JavaAudioDeviceModule.builder(this._appContext)).thenReturn(this._audioBuilder);

    this._pcFactoryBuilder = mock(PeerConnectionFactory.Builder.class);
    when(this._pcFactoryBuilder.createPeerConnectionFactory()).thenReturn(this._pcFactory);
    when(this._pcFactoryBuilder.setVideoDecoderFactory(this._decoderFactory)).thenReturn(this._pcFactoryBuilder);
    when(this._pcFactoryBuilder.setVideoEncoderFactory(this._encoderFactory)).thenReturn(this._pcFactoryBuilder);
    when(this._pcFactoryBuilder.setAudioDeviceModule(this._audioDeviceModule)).thenReturn(this._pcFactoryBuilder);
    when(PeerConnectionFactory.builder()).thenReturn(this._pcFactoryBuilder);
  }

  @Test
  public void itShouldInitializeThePeerConnectionFactoryOnCreate() {
    new PeerFactoryImpl(this._conf, this._delegate, this._appContext, this._eglBase);

    verifyStatic(PeerConnectionFactory.class);
    PeerConnectionFactory.initialize(this._options);
  }

  @Test
  public void itShouldInitializeTheCamera2EnumeratorIfSupportedOnCreate() throws Exception {
    new PeerFactoryImpl(this._conf, this._delegate, this._appContext, this._eglBase);
    verifyNew(Camera2Enumerator.class).withArguments(this._appContext);
  }

  @Test
  public void itShouldInitializeTheCamera1EnumeratorIfCamera2IsNotSupportedOnCreate() throws Exception {
    when(Camera2Enumerator.isSupported(this._appContext)).thenReturn(false);

    new PeerFactoryImpl(this._conf, this._delegate, this._appContext, this._eglBase);
    verifyNew(Camera1Enumerator.class).withArguments(false);
  }

  @Test
  public void itShouldCreateAPeerconnection() {
    PeerFactoryImpl factory = new PeerFactoryImpl(this._conf, this._delegate, this._appContext, this._eglBase);

    Protocol owner = mock(Protocol.class);
    assertNotNull(factory.create(-1, owner));

    verify(this._pcFactoryBuilder).setVideoDecoderFactory(this._decoderFactory);
    verify(this._pcFactoryBuilder).setVideoEncoderFactory(this._encoderFactory);
    verify(this._pcFactoryBuilder).setAudioDeviceModule(this._audioDeviceModule);
  }

}