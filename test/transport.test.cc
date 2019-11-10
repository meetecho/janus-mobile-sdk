#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "janus/transport.h"

#include "mocks/transport_delegate.h"
#include "mocks/http_factory.h"
#include "mocks/http.h"
#include "mocks/async.h"
#include "mocks/matchers.h"

using testing::NiceMock;
using testing::Return;
using testing::_;
using testing::Eq;
using testing::Invoke;
using testing::IsJsonEq;
using testing::InSequence;

namespace Janus {

  void noop(Task task) {}

  void callback(Task task) {
    task();
  }

  class HttpTransportTest : public testing::Test {
    protected:
      void SetUp() override {
        this->_delegate = std::make_shared<NiceMock<TransportDelegateMock>>();

        this->_reply = {
          { "janus", "test reply" }
        };
        this->_client = std::make_shared<NiceMock<HttpMock>>();
        auto httpReply = std::make_shared<HttpResponse>(200, this->_reply.dump());
        ON_CALL(*this->_client, get(_)).WillByDefault(Return(httpReply));
        ON_CALL(*this->_client, post(_, _)).WillByDefault(Return(httpReply));

        this->_factory = std::make_shared<NiceMock<HttpFactoryMock>>();
        ON_CALL(*this->_factory, create("http://base")).WillByDefault(Return(this->_client));

        this->_async = std::make_shared<NiceMock<AsyncMock>>();
        ON_CALL(*this->_async, submit(_)).WillByDefault(Invoke(callback));
      }

      std::shared_ptr<NiceMock<TransportDelegateMock>> _delegate;
      std::shared_ptr<NiceMock<HttpFactoryMock>> _factory;
      std::shared_ptr<NiceMock<HttpMock>> _client;
      std::shared_ptr<NiceMock<AsyncMock>> _async;
      nlohmann::json _reply;
  };

  TEST_F(HttpTransportTest, shouldAllocateAPoolOfClients) {
    EXPECT_CALL(*this->_factory, create("http://base")).Times(2);

    auto httpTransport = std::make_shared<HttpTransport>("http://base", this->_delegate, this->_factory, this->_async);
  }

  TEST_F(HttpTransportTest, shouldSendPostRequestToSendBody) {
    auto bundle = Bundle::create();

    nlohmann::json request = {
      { "janus", "test request" }
    };
    EXPECT_CALL(*this->_client, post("/", request.dump())).Times(1);
    EXPECT_CALL(*this->_delegate, onMessage(IsJsonEq(this->_reply), Eq(bundle))).Times(1);

    auto httpTransport = std::make_shared<HttpTransport>("http://base", this->_delegate, this->_factory, this->_async);
    httpTransport->send(request, bundle);
  }

  TEST_F(HttpTransportTest, shouldAppendTheSessionIdIfSet) {
    nlohmann::json request = {
      { "janus", "test request" }
    };
    EXPECT_CALL(*this->_client, post("/session-id", request.dump())).Times(1);

    auto async = std::make_shared<NiceMock<AsyncMock>>();
    ON_CALL(*async, submit(_)).WillByDefault(Invoke(noop));

    {
      InSequence sequence;

      EXPECT_CALL(*async, submit(_)).WillOnce(Invoke(noop));
      EXPECT_CALL(*async, submit(_)).WillOnce(Invoke(callback));
    }

    auto httpTransport = std::make_shared<HttpTransport>("http://base", this->_delegate, this->_factory, async);
    httpTransport->sessionId("session-id");

    httpTransport->send(request, Bundle::create());
  }

  TEST_F(HttpTransportTest, shouldStartLongPollingOnSessionIdSet) {
    EXPECT_CALL(*this->_client, get("/session-id")).Times(1);
    EXPECT_CALL(*this->_delegate, onMessage(IsJsonEq(this->_reply), _)).Times(1);

    auto async = std::make_shared<NiceMock<AsyncMock>>();

    {
      InSequence sequence;

      EXPECT_CALL(*async, submit(_)).WillOnce(Invoke(callback));
      EXPECT_CALL(*async, submit(_)).WillOnce(Invoke(noop));
    }

    auto httpTransport = std::make_shared<HttpTransport>("http://base", this->_delegate, this->_factory, async);
    httpTransport->sessionId("session-id");
  }

  TEST_F(HttpTransportTest, shouldDisableLongPollingOnClose) {
    EXPECT_CALL(*this->_client, get("/session-id")).Times(0);

    auto httpTransport = std::make_shared<HttpTransport>("http://base", this->_delegate, this->_factory, this->_async);
    httpTransport->close();
    httpTransport->sessionId("session-id");
  }

  TEST_F(HttpTransportTest, shouldDisableSendOnClose) {
    EXPECT_CALL(*this->_client, post(_, _)).Times(0);
    nlohmann::json request = {
      { "janus", "test request" }
    };

    auto httpTransport = std::make_shared<HttpTransport>("http://base", this->_delegate, this->_factory, this->_async);
    httpTransport->close();
    httpTransport->send(request, Bundle::create());
  }


  class TransportFactoryTest : public testing::Test {
    protected:
      void SetUp() override {
        this->_async = std::make_shared<NiceMock<AsyncMock>>();
        this->_delegate = std::make_shared<NiceMock<TransportDelegateMock>>();
      }

      std::shared_ptr<NiceMock<TransportDelegateMock>> _delegate;
      std::shared_ptr<NiceMock<AsyncMock>> _async;
  };

  TEST_F(TransportFactoryTest, shouldCreateATransportByCheckingTheURLProtocol) {
    auto factory = std::make_shared<TransportFactoryImpl>();

    auto http = factory->create("http://yolo", this->_delegate);
    EXPECT_NE(http, nullptr);
    EXPECT_EQ(http->type(), TransportType::HTTP);

    auto https = factory->create("https://yolo", this->_delegate);
    EXPECT_NE(https, nullptr);
    EXPECT_EQ(https->type(), TransportType::HTTP);

    auto ws = factory->create("ws://yolo", this->_delegate);
    EXPECT_NE(ws, nullptr);
    EXPECT_EQ(ws->type(), TransportType::WS);

    auto wss = factory->create("wss://yolo", this->_delegate);
    EXPECT_NE(wss, nullptr);
    EXPECT_EQ(wss->type(), TransportType::WS);
  }

}
