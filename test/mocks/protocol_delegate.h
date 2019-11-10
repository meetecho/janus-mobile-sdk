#pragma once

#include "janus/protocol_delegate.hpp"

namespace Janus {

  class ProtocolDelegateMock : public ProtocolDelegate {
    public:
      MOCK_METHOD0(onReady, void());
      MOCK_METHOD2(onError, void(const JanusError& error, const std::shared_ptr<Bundle>& context));
      MOCK_METHOD2(onEvent, void(const std::shared_ptr<JanusEvent>& event, const std::shared_ptr<Bundle>& context));
      MOCK_METHOD1(onHangup, void(const std::string& reason));
      MOCK_METHOD0(onClose, void());
  };

}
