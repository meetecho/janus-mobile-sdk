package com.github.helloiampau.app.custom;

import com.github.helloiampau.janus.generated.Bundle;
import com.github.helloiampau.janus.generated.JanusEvent;
import com.github.helloiampau.janus.generated.Plugin;

public class CustomPlugin extends Plugin {

  @Override
  public void onEvent(JanusEvent event, Bundle context) {

  }

  @Override
  public void onHangup(String reason) {

  }

  @Override
  public void onClose() {

  }

  @Override
  public void command(String command, Bundle payload) {

  }

  @Override
  public void onOffer(String sdp, Bundle context) {

  }

  @Override
  public void onAnswer(String sdp, Bundle context) {

  }

}
