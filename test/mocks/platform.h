#pragma once

#include "janus/platform_impl.h"

namespace Janus {

  class PlatformMock : public PlatformImpl {
    public:
      MOCK_METHOD1(protocol, void(const std::shared_ptr<Protocol>& protocol));
      MOCK_METHOD0(protocol, std::shared_ptr<Protocol>());

      MOCK_METHOD2(pluginFactory, void(const std::string& id, const std::shared_ptr<PluginFactory>& factory));
      MOCK_METHOD3(plugin, std::shared_ptr<Plugin>(const std::string& id, int64_t handleId, const std::shared_ptr<Protocol>& owner));
  };

}
