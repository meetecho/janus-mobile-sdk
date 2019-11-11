package com.github.helloiampau.app.custom;

import com.github.helloiampau.janus.generated.Plugin;
import com.github.helloiampau.janus.generated.PluginFactory;
import com.github.helloiampau.janus.generated.Protocol;

public class CustomPluginFactory extends PluginFactory {

  @Override
  public Plugin create(long handleId, Protocol owner) {
    return new CustomPlugin();
  }
}
