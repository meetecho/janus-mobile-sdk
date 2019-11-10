#pragma once

#include "janus/transport.h"

namespace Janus {

  class TransportMock : public Transport {
    public:
      MOCK_METHOD0(type, TransportType());
      MOCK_METHOD2(send, void(const nlohmann::json& message, const std::shared_ptr<Bundle>& context));
      MOCK_METHOD1(sessionId, void(const std::string& sessionId));
      MOCK_METHOD0(close, void());
  };

}
