/*!
 * janus-client SDK
 *
 * transport.h
 * Janus platform implementation
 * This class holds references to custom protocol and plugins
 *
 * Copyright 2019 Pasquale Boemio <pau@helloiampau.io>
 */

#pragma once

#include <unordered_map>

#include "janus/platform.hpp"
#include "janus/plugin.hpp"
#include "janus/plugin_factory.hpp"

namespace Janus {

  class PlatformImpl : public Platform {
    public:
      virtual std::shared_ptr<Protocol> protocol() = 0;
      virtual std::shared_ptr<Plugin> plugin(const std::string& id, int64_t handleId, const std::shared_ptr<Protocol>& owner) = 0;
  };

  class PlatformImplImpl : public PlatformImpl {
    public:
      PlatformImplImpl(const std::shared_ptr<PeerFactory>& factory);

      void protocol(const std::shared_ptr<Protocol>& protocol);
      std::shared_ptr<Protocol> protocol();

      void pluginFactory(const std::string& id, const std::shared_ptr<PluginFactory>& factory);
      std::shared_ptr<Plugin> plugin(const std::string& id, int64_t handleId, const std::shared_ptr<Protocol>& owner);

      std::shared_ptr<PeerFactory> peerFactory();

    private:
      std::shared_ptr<Protocol> _protocol;
      std::unordered_map<std::string, std::shared_ptr<PluginFactory>> _factories;
      std::shared_ptr<PeerFactory> _peerFactory;
  };

}
