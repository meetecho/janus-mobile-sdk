package com.github.helloiampau.app.custom;

import com.github.helloiampau.janus.generated.Bundle;
import com.github.helloiampau.janus.generated.JanusConf;
import com.github.helloiampau.janus.generated.Platform;
import com.github.helloiampau.janus.generated.Protocol;
import com.github.helloiampau.janus.generated.ProtocolDelegate;

public class CustomProtocol extends Protocol {

  @Override
  public String name() {
    return "My Yolo Protocol";
  }

  @Override
  public void init(JanusConf conf, Platform platform, ProtocolDelegate delegate) {

  }

  @Override
  public void dispatch(String command, Bundle payload) {

  }

  @Override
  public void hangup() {

  }

  @Override
  public void close() {

  }

  @Override
  public void onOffer(String sdp, Bundle context) {

  }

  @Override
  public void onAnswer(String sdp, Bundle context) {

  }

  @Override
  public void onIceCandidate(String mid, int index, String sdp, long id) {

  }

  @Override
  public void onIceCompleted(long id) {

  }

}
