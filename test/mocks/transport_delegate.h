#pragma once

#include "janus/transport.h"

namespace Janus {

  class TransportDelegateMock : public TransportDelegate {
    public:
      MOCK_METHOD2(onMessage, void(const nlohmann::json& message, const std::shared_ptr<Bundle>& context));
  };

}
