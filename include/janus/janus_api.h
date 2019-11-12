/*!
 * janus-client SDK
 *
 * janus_api.h
 * The JanusApi
 * This module implements the janus API: the default signaling protocol of this SDK
 *
 * Copyright 2019 Pasquale Boemio <pau@helloiampau.io>
 */

#pragma once

#include "janus/protocol.hpp"

#include "janus/transport.h"
#include "janus/janus_conf.hpp"
#include "janus/protocol_delegate.hpp"
#include "janus/random.h"
#include "janus/platform_impl.h"
#include "janus/plugin.hpp"
#include "janus/janus_event_impl.h"

#define JANUS_API "Janus API"

namespace Janus {

  enum ReadyState {
    CLOSED,
    INIT,
    READY,
    CLOSING
  };

  class PluginCommandDelegate {
    public:
      virtual void onCommandResult(const nlohmann::json& body, const std::shared_ptr<Bundle>& context) = 0;
      virtual void onPluginEvent(const std::shared_ptr<JanusEvent>& event, const std::shared_ptr<Bundle>& context) = 0;
  };

  class JanusApi : public Protocol, public TransportDelegate, public PluginCommandDelegate, public std::enable_shared_from_this<JanusApi> {
    public:
      std::string name() {
        return JANUS_API;
      }

      JanusApi(const std::shared_ptr<Random>& random, const std::shared_ptr<TransportFactory>& transportFactory);
      ~JanusApi();

      void init(const std::shared_ptr<JanusConf>& conf, const std::shared_ptr<Platform>& platform, const std::shared_ptr<ProtocolDelegate>& delegate);
      void close();
      void hangup();
      void dispatch(const std::string& command, const std::shared_ptr<Bundle>& payload);

      void onMessage(const nlohmann::json& message, const std::shared_ptr<Bundle>& context);

      void onOffer(const std::string& sdp, const std::shared_ptr<Bundle>& context);
      void onAnswer(const std::string& sdp, const std::shared_ptr<Bundle>& context);
      void onIceCandidate(const std::string& mid, int32_t index, const std::string& sdp, int64_t id);
      void onIceCompleted(int64_t id);

      void onCommandResult(const nlohmann::json& body, const std::shared_ptr<Bundle>& context);
      void onPluginEvent(const std::shared_ptr<JanusEvent>& event, const std::shared_ptr<Bundle>& context);

      int64_t handleId(const std::shared_ptr<Bundle>& context);

    private:
      ReadyState readyState();
      void readyState(ReadyState readyState);

      int64_t _handleId = -1;

      std::shared_ptr<Plugin> _plugin = nullptr;
      std::shared_ptr<PlatformImpl> _platform;
      std::shared_ptr<TransportFactory> _transportFactory;
      std::shared_ptr<Transport> _transport;
      std::shared_ptr<Random> _random;
      std::shared_ptr<ProtocolDelegate> _delegate;

      std::mutex _readyStateMutex;
      ReadyState _readyState = ReadyState::CLOSED;
  };

}
