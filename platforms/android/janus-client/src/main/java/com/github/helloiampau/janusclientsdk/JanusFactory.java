package com.github.helloiampau.janusclientsdk;

import android.content.Context;

import com.github.helloiampau.janus.generated.Janus;
import com.github.helloiampau.janus.generated.JanusConf;
import com.github.helloiampau.janus.generated.PeerFactory;
import com.github.helloiampau.janus.generated.Platform;
import com.github.helloiampau.janus.generated.PluginFactory;
import com.github.helloiampau.janus.generated.Protocol;
import com.github.helloiampau.janusclientsdk.rtc.PeerFactoryImpl;

import org.webrtc.EglBase;

import java.util.HashMap;
import java.util.Map;

public class JanusFactory {

  static {
    System.loadLibrary("janus-android-sdk");
  }

  private EglBase _rootEglBase;
  private Protocol _protocol;
  private Map<String, PluginFactory> _plugins = new HashMap<>();

  public JanusFactory() {
    this._rootEglBase = EglBase.create();
  }

  public EglBase rootEgl() {
    return this._rootEglBase;
  }

  public void protocol(Protocol protocol) {
    this._protocol = protocol;
  }

  public void pluginFactory(String id, PluginFactory factory) {
    this._plugins.put(id, factory);
  }

  public Janus create(JanusConf conf, JanusDelegate delegate, Context appContext) {
    PeerFactory factory = new PeerFactoryImpl(conf, delegate, appContext, this.rootEgl());
    Platform platform = Platform.create(factory);

    if(this._protocol != null) {
      platform.protocol(this._protocol);
    }

    for(String id : this._plugins.keySet()) {
      platform.pluginFactory(id, this._plugins.get(id));
    }

    return Janus.create(conf, platform, delegate);
  }

}
