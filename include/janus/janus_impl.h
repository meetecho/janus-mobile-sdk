/*!
 * janus-client SDK
 *
 * janus_impl.h
 * Janus service implementation
 * This class defines the main API you use to interact with the library
 *
 * Copyright 2019 Pasquale Boemio <pau@helloiampau.io>
 */

#pragma once

#include "janus/janus.hpp"

#include "janus/platform_impl.h"
#include "janus/protocol_delegate.hpp"

namespace Janus {

  class JanusImpl : public Janus {
    public:
      JanusImpl(const std::shared_ptr<JanusConf>& conf, const std::shared_ptr<Platform>& platform, const std::shared_ptr<ProtocolDelegate>& delegate);

      void init();
      void close();
      void hangup();
      void dispatch(const std::string& command, const std::shared_ptr<Bundle>& payload);

    private:
      std::shared_ptr<JanusConf> _conf;
      std::shared_ptr<PlatformImpl> _platform;
      std::shared_ptr<ProtocolDelegate> _delegate;

      std::shared_ptr<Protocol> _protocol;
  };

}
