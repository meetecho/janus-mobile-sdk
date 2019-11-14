#include "janus/platform_impl.h"

#include "janus/janus_api.h"
#include "janus/random.h"

#include "janus/janus_plugins.hpp"
#include "janus/plugins/janus_plugin_echotest.h"
#include "janus/plugins/janus_plugin_streaming.h"
#include "janus/plugins/janus_plugin_videoroom.h"

namespace Janus {

  /* PlatformImplImpl */

  PlatformImplImpl::PlatformImplImpl(const std::shared_ptr<PeerFactory>& factory) {
    auto transportFactory = std::make_shared<TransportFactoryImpl>();
    auto random = std::make_shared<RandomImpl>();

    auto protocol = std::make_shared<JanusApi>(random, transportFactory);
    this->protocol(protocol);

    auto echotestFactory = std::make_shared<JanusPluginEchotestFactory>(protocol, factory);
    this->pluginFactory(JanusPlugins::ECHO_TEST, echotestFactory);

    auto streamingFactory = std::make_shared<JanusPluginStreamingFactory>(protocol, factory);
    this->pluginFactory(JanusPlugins::STREAMING, streamingFactory);

    auto videoroomFactory = std::make_shared<JanusPluginVideoroomFactory>(protocol, factory);
    this->pluginFactory(JanusPlugins::VIDEOROOM, videoroomFactory);

    this->_peerFactory = factory;
  }

  void PlatformImplImpl::protocol(const std::shared_ptr<Protocol>& protocol) {
    this->_protocol = protocol;
  }

  std::shared_ptr<Protocol> PlatformImplImpl::protocol() {
    return this->_protocol;
  }

  void PlatformImplImpl::pluginFactory(const std::string& id, const std::shared_ptr<PluginFactory>& factory) {
    this->_factories[id] = factory;
  }

  std::shared_ptr<Plugin> PlatformImplImpl::plugin(const std::string& id, int64_t handleId, const std::shared_ptr<Protocol>& owner) {
    auto plugin = this->_factories[id]->create(handleId, owner);

    return plugin;
  }

  std::shared_ptr<PeerFactory> PlatformImplImpl::peerFactory() {
    return this->_peerFactory;
  }

  /* Platform */

  std::shared_ptr<Platform> Platform::create(const std::shared_ptr<PeerFactory>& factory) {
    auto instance = std::make_shared<PlatformImplImpl>(factory);

    return instance;
  }

}
