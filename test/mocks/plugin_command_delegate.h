#pragma once

#include "janus/janus_api.h"

namespace Janus {

  class PluginCommandDelegateMock : public PluginCommandDelegate {
    public:
      MOCK_METHOD2(onCommandResult, void(const nlohmann::json& body, const std::shared_ptr<Bundle>& context));
      MOCK_METHOD2(onPluginEvent, void(const std::shared_ptr<JanusEvent>& event, const std::shared_ptr<Bundle>& context));
  };

};
