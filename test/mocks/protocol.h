#pragma once

#include "janus/protocol.hpp"

namespace Janus {

  class ProtocolMock : public Protocol {
    public:
      MOCK_METHOD0(name, std::string());
      MOCK_METHOD0(close, void());
      MOCK_METHOD0(hangup, void());
      MOCK_METHOD3(init, void(const std::shared_ptr<JanusConf>& conf, const std::shared_ptr<Platform>& platform, const std::shared_ptr<ProtocolDelegate>& delegate));
      MOCK_METHOD2(dispatch, void(const std::string& command, const std::shared_ptr<Bundle>& payload));

      MOCK_METHOD2(onOffer, void(const std::string& sdp, const std::shared_ptr<Bundle>& context));
      MOCK_METHOD2(onAnswer, void(const std::string& sdp, const std::shared_ptr<Bundle>& context));
      MOCK_METHOD4(onIceCandidate, void(const std::string& mid, int32_t index, const std::string& sdp, int64_t id));
      MOCK_METHOD1(onIceCompleted, void(int64_t id));
  };

}
