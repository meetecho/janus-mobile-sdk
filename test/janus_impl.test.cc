#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "janus/janus_impl.h"

#include "janus/janus_error.hpp"

#include "mocks/protocol_delegate.h"
#include "mocks/platform.h"
#include "mocks/protocol.h"
#include "mocks/bundle.h"
#include "mocks/janus_conf.h"

using testing::NiceMock;
using testing::Return;
using testing::Eq;
using testing::_;

namespace Janus {

  class JanusImplTest : public testing::Test {
    protected:
      void SetUp() override {
        this->_delegate = std::make_shared<NiceMock<ProtocolDelegateMock>>();
        this->_protocol = std::make_shared<NiceMock<ProtocolMock>>();
        this->_payload = std::make_shared<NiceMock<BundleMock>>();
        this->_conf = std::make_shared<NiceMock<JanusConfMock>>();

        this->_platform = std::make_shared<NiceMock<PlatformMock>>();
        ON_CALL(*this->_platform, protocol()).WillByDefault(Return(this->_protocol));
      }

      std::shared_ptr<NiceMock<JanusConfMock>> _conf;
      std::shared_ptr<NiceMock<ProtocolDelegateMock>> _delegate;
      std::shared_ptr<NiceMock<PlatformMock>> _platform;
      std::shared_ptr<NiceMock<ProtocolMock>> _protocol;
      std::shared_ptr<NiceMock<BundleMock>> _payload;
  };

  TEST_F(JanusImplTest, shouldDelegateInitToProtocolOncePerSession) {
    // if we check on the platform argument here, we got a memory leak caused by the ON_CALL assertion defined on the same smart pointer. We need to open an issue on Google Test project.
    // EXPECT_CALL(*this->_protocol, init(_ Eq(this->_platform), Eq(this->_delegate))).Times(1);
    EXPECT_CALL(*this->_protocol, init(_, _, Eq(this->_delegate))).Times(1);

    auto janus = std::make_shared<JanusImpl>(this->_conf, this->_platform, this->_delegate);
    janus->init();
    janus->init();
  }

  TEST_F(JanusImplTest, shouldDelegateDispatchToProtocol) {
    EXPECT_CALL(*this->_protocol, dispatch("my command", Eq(this->_payload)));

    auto janus = std::make_shared<JanusImpl>(this->_conf, this->_platform, this->_delegate);
    janus->dispatch("my command", this->_payload);
  }

  TEST_F(JanusImplTest, shouldDelegateTheCloseCommandToProtocol) {
    EXPECT_CALL(*this->_protocol, close());

    auto janus = std::make_shared<JanusImpl>(this->_conf, this->_platform, this->_delegate);
    janus->init();
    janus->close();
  }

  TEST_F(JanusImplTest, shouldSkipCloseIfProtocolIsNotInitialized) {
    auto janus = std::make_shared<JanusImpl>(this->_conf, this->_platform, this->_delegate);
    janus->close();
  }

  TEST_F(JanusImplTest, shouldResetTheProtocolAfterClose) {
    EXPECT_CALL(*this->_protocol, init(_, _, Eq(this->_delegate))).Times(2);

    auto janus = std::make_shared<JanusImpl>(this->_conf, this->_platform, this->_delegate);
    janus->init();
    janus->close();
    janus->init();
  }

  TEST_F(JanusImplTest, shouldDelegateTheHangupCommandToProtocol) {
    EXPECT_CALL(*this->_protocol, hangup());

    auto janus = std::make_shared<JanusImpl>(this->_conf, this->_platform, this->_delegate);
    janus->init();
    janus->hangup();
  }

  TEST_F(JanusImplTest, shouldSkipHangupIfProtocolIsNotInitialized) {
    auto janus = std::make_shared<JanusImpl>(this->_conf, this->_platform, this->_delegate);
    janus->hangup();
  }

  class JanusTest : public testing::Test {
    protected:
      void SetUp() override {
        this->_delegate = std::make_shared<NiceMock<ProtocolDelegateMock>>();
        this->_platform = std::make_shared<NiceMock<PlatformMock>>();
        this->_conf = std::make_shared<NiceMock<JanusConfMock>>();
      }

      std::shared_ptr<NiceMock<JanusConfMock>> _conf;
      std::shared_ptr<NiceMock<ProtocolDelegateMock>> _delegate;
      std::shared_ptr<NiceMock<PlatformMock>> _platform;
  };

  TEST_F(JanusTest, shouldCreateAJanusImplObject) {
    auto service = Janus::create(this->_conf, this->_platform, this->_delegate);
    EXPECT_NE(service, nullptr);
  }

}
