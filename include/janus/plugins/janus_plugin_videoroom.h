/*!
 * janus-client SDK
 *
 * janus_plugin_videoroom.h
 * The Janus Videoroom Plugin
 * This module implements the janus videoroom plugin client-side interface.
 *
 * Copyright 2019 Pasquale Boemio <pau@helloiampau.io>
 */

#pragma once

#include <unordered_map>

#include "janus/plugins/janus_plugin.h"
#include "janus/janus_plugins.hpp"

namespace Janus {

  struct Subscriber {
    std::shared_ptr<Peer> peer;
    std::shared_ptr<Bundle> context;

    Subscriber(std::shared_ptr<Peer> peer_, std::shared_ptr<Bundle> context_) : peer(std::move(peer_)), context(std::move(context_)) {}
  };

  class JanusPluginVideoroom : public JanusPlugin {
    public:
      JanusPluginVideoroom(int64_t handleId, const std::shared_ptr<PluginCommandDelegate>& delegate, const std::shared_ptr<PeerFactory>& peerFactory, const std::shared_ptr<Protocol>& owner) : JanusPlugin(handleId, delegate, peerFactory, owner) {}
      void command(const std::string& command, const std::shared_ptr<Bundle>& payload);
      void onEvent(const std::shared_ptr<JanusEvent>& event, const std::shared_ptr<Bundle>& context);
      void onOffer(const std::string& sdp, const std::shared_ptr<Bundle>& context);
      void onAnswer(const std::string& sdp, const std::shared_ptr<Bundle>& context);

      std::string name() {
        return JanusPlugins::VIDEOROOM;
      }

    private:
      std::unordered_map<int64_t, std::shared_ptr<Subscriber>> _subscribers;
  };

  class JanusPluginVideoroomFactory : public PluginFactory {
    public:
      JanusPluginVideoroomFactory(const std::shared_ptr<PluginCommandDelegate>& delegate, const std::shared_ptr<PeerFactory>& peerFactory);

      std::shared_ptr<Plugin> create(int64_t handleId, const std::shared_ptr<Protocol>& owner);

    private:
      std::shared_ptr<PeerFactory> _peerFactory;
      std::shared_ptr<PluginCommandDelegate> _delegate;
  };

}
