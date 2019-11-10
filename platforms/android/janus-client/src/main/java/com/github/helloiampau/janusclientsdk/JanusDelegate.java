package com.github.helloiampau.janusclientsdk;

import com.github.helloiampau.janus.generated.ProtocolDelegate;
import com.github.helloiampau.janusclientsdk.rtc.MediaBundle;

import org.webrtc.DataChannel;

public abstract class JanusDelegate extends ProtocolDelegate {

  public abstract void onMediaChanged(MediaBundle media);
  public abstract void onDatachannelMessage(DataChannel.Buffer buffer);
  public abstract String onOffer(String sdp);
  public abstract String onAnswer(String sdp);

}
