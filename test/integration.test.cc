#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <mutex>
#include <condition_variable>

#include "janus/platform.hpp"
#include "janus/protocol_delegate.hpp"
#include "janus/janus.hpp"

#include "mocks/peer_factory.h"
#include "mocks/peer.h"
#include "mocks/janus_conf.h"

using testing::NiceMock;
using testing::Return;

namespace Janus {

  class ApiTest : public testing::Test {
  };

  class Delegate : public ProtocolDelegate {
    public:
      void onReady() {
        this->condition.notify_one();
      }

      void onClose() {
        this->condition.notify_one();
      }

      void onError(const JanusError& error, const std::shared_ptr<Bundle>& context) {}
      void onEvent(const std::shared_ptr<JanusEvent>& event, const std::shared_ptr<Bundle>& context) {}
      void onHangup(const std::string& reason) {}

      std::mutex mutex;
      std::condition_variable condition;
  };

  TEST_F(ApiTest, shouldCreateANewSession) {
    auto peer = std::make_shared<NiceMock<PeerMock>>();
    auto factory = std::make_shared<NiceMock<PeerFactoryMock>>();

    auto conf = std::make_shared<NiceMock<JanusConfMock>>();
    ON_CALL(*conf, url()).WillByDefault(Return("http://janus:8088/janus"));
    ON_CALL(*conf, plugin()).WillByDefault(Return("janus.plugin.echotest"));

    auto delegate = std::make_shared<Delegate>();

    auto platform = Platform::create(factory);
    auto janus = Janus::create(conf, platform, delegate);

    janus->init();

    {
      std::unique_lock<std::mutex> lock(delegate->mutex);
      delegate->condition.wait(lock);
    }

    janus->close();

    {
      std::unique_lock<std::mutex> lock(delegate->mutex);
      delegate->condition.wait(lock);
    }
  }

}
