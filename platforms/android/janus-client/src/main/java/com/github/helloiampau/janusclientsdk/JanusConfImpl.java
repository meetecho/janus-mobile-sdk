package com.github.helloiampau.janusclientsdk;

import com.github.helloiampau.janus.generated.JanusConf;

public class JanusConfImpl extends JanusConf {
  private String _url;

  @Override
  public String url() {
    return this._url;
  }

  public void url(String url) {
    this._url = url;
  }

  private String _plugin;

  @Override
  public String plugin() {
    return this._plugin;
  }

  public void plugin(String plugin) {
    this._plugin = plugin;
  }

}
