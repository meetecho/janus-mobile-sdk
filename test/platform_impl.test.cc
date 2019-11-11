#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "janus/platform_impl.h"

#include "janus/janus_plugins.hpp"
#include "janus/janus_api.h"

#include "mocks/protocol.h"
#include "mocks/peer_factory.h"

using testing::NiceMock;

namespace Janus {

  class PlatformImplTest : public testing::Test {
  };

  TEST_F(PlatformImplTest, shouldStoreAndRetrieveAProtocolObject) {
    auto protocol = std::make_shared<NiceMock<ProtocolMock>>();

    auto peerFactory = std::make_shared<NiceMock<PeerFactoryMock>>();

    auto platform = std::make_shared<PlatformImplImpl>(peerFactory);
    platform->protocol(protocol);

    EXPECT_EQ(platform->protocol(), protocol);
  }

  TEST_F(PlatformImplTest, shouldInitializeItselfWithTheJanusAPI) {
    auto peerFactory = std::make_shared<NiceMock<PeerFactoryMock>>();
    auto platform = std::make_shared<PlatformImplImpl>(peerFactory);

    EXPECT_EQ(platform->protocol()->name(), JANUS_API);
  }

  TEST_F(PlatformImplTest, shouldRegisterTheStreamingFactory) {
    auto peerFactory = std::make_shared<NiceMock<PeerFactoryMock>>();
    auto owner = std::make_shared<NiceMock<ProtocolMock>>();

    auto platform = std::make_shared<PlatformImplImpl>(peerFactory);
    platform->plugin(JanusPlugins::STREAMING, 69, owner);
  }

  TEST_F(PlatformImplTest, shouldRegisterTheVideoroomFactory) {
    auto peerFactory = std::make_shared<NiceMock<PeerFactoryMock>>();
    auto owner = std::make_shared<NiceMock<ProtocolMock>>();

    auto platform = std::make_shared<PlatformImplImpl>(peerFactory);
    platform->plugin(JanusPlugins::VIDEOROOM, 69, owner);
  }

  TEST_F(PlatformImplTest, shouldRegisterTheEchotestFactory) {
    auto peerFactory = std::make_shared<NiceMock<PeerFactoryMock>>();
    auto owner = std::make_shared<NiceMock<ProtocolMock>>();

    auto platform = std::make_shared<PlatformImplImpl>(peerFactory);
    platform->plugin(JanusPlugins::ECHO_TEST, 69, owner);
  }

  class PlatformTest : public testing::Test {
    protected:
      void SetUp() override {
        this->_factory = std::make_shared<NiceMock<PeerFactoryMock>>();
      }

      std::shared_ptr<NiceMock<PeerFactoryMock>> _factory;
  };

  TEST_F(PlatformTest, shouldCreateAPlatformImplObject) {
    auto platform = Platform::create(this->_factory);
    EXPECT_NE(platform, nullptr);
  }

}
