/*!
 * janus-client SDK
 *
 * janus_plugin_echotest.h
 * The Janus Echotest Plugin
 * This module implements the janus echotest plugin client-side interface.
 *
 * Copyright 2019 Pasquale Boemio <pau@helloiampau.io>
 */

#pragma once

#include "janus/plugins/janus_plugin.h"

#include "janus/janus_plugins.hpp"
#include "janus/plugin_factory.hpp"
#include "janus/janus_event_impl.h"

namespace Janus {

  class JanusPluginEchotest : public JanusPlugin {
    public:
      JanusPluginEchotest(int64_t handleId, const std::shared_ptr<PluginCommandDelegate>& delegate, const std::shared_ptr<PeerFactory>& peerFactory, const std::shared_ptr<Protocol>& owner) : JanusPlugin(handleId, delegate, peerFactory, owner) {}
      void command(const std::string& command, const std::shared_ptr<Bundle>& payload);
      void onEvent(const std::shared_ptr<JanusEvent>& event, const std::shared_ptr<Bundle>& context);
      void onOffer(const std::string& sdp, const std::shared_ptr<Bundle>& context);
      void onAnswer(const std::string& sdp, const std::shared_ptr<Bundle>& context) {}

      std::string name() {
        return JanusPlugins::ECHO_TEST;
      }
  };

  class JanusPluginEchotestFactory : public PluginFactory {
    public:
      JanusPluginEchotestFactory(const std::shared_ptr<PluginCommandDelegate>& delegate, const std::shared_ptr<PeerFactory>& peerFactory);

      std::shared_ptr<Plugin> create(int64_t handleId, const std::shared_ptr<Protocol>& owner);

    private:
      std::shared_ptr<PeerFactory> _peerFactory;
      std::shared_ptr<PluginCommandDelegate> _delegate;
  };

};
