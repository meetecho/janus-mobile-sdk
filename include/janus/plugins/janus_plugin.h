/*!
 * janus-client SDK
 *
 * janus_plugin.h
 * The Janus Base Plugin
 * This module implements the base functionalities of all janus plugins.
 *
 * Copyright 2019 Pasquale Boemio <pau@helloiampau.io>
 */

#pragma once

#include "janus/plugin.hpp"

#include "janus/peer.hpp"
#include "janus/peer_factory.hpp"
#include "janus/janus_api.h"

namespace Janus {

  class JanusPlugin : public Plugin {
    public:
      JanusPlugin(int64_t handleId, const std::shared_ptr<PluginCommandDelegate>& delegate, const std::shared_ptr<PeerFactory>& peerFactory, const std::shared_ptr<Protocol>& owner);
      void onHangup(const std::string& reason);
      void onClose();

    protected:
      std::shared_ptr<Peer> _peer;

      int64_t _handleId = -1;

      std::shared_ptr<Protocol> _owner;
      std::shared_ptr<PeerFactory> _peerFactory;
      std::shared_ptr<PluginCommandDelegate> _delegate;
  };

}
