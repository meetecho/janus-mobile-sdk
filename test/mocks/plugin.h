#pragma once

#include "janus/plugin.hpp"

namespace Janus {

  class PluginMock : public Plugin {
    public:
      MOCK_METHOD0(onClose, void());
      MOCK_METHOD1(onHangup, void(const std::string& reason));
      MOCK_METHOD2(onEvent, void(const std::shared_ptr<JanusEvent>& event, const std::shared_ptr<Bundle>& context));
      MOCK_METHOD2(command, void(const std::string& command, const std::shared_ptr<Bundle>& payload));
      MOCK_METHOD2(onOffer, void(const std::string& sdp, const std::shared_ptr<Bundle>& context));
      MOCK_METHOD2(onAnswer, void(const std::string& sdp, const std::shared_ptr<Bundle>& context));
  };

}
