#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "janus/janus_api.h"

#include "janus/janus_error.hpp"

#include "mocks/transport_factory.h"
#include "mocks/transport.h"
#include "mocks/protocol_delegate.h"
#include "mocks/platform.h"
#include "mocks/random.h"
#include "mocks/matchers.h"
#include "mocks/janus_conf.h"
#include "mocks/plugin.h"

using testing::NiceMock;
using testing::_;
using testing::Eq;
using testing::Return;
using testing::IsJsonEq;
using testing::IsJanusMessage;
using testing::BundleHasString;
using testing::InSequence;
using testing::IsEvent;
using testing::HasJsep;
using testing::IsError;

namespace Janus {

  class JanusApiTest : public testing::Test {
    protected:
      void SetUp() override {
        this->_random = std::make_shared<NiceMock<RandomMock>>();
        ON_CALL(*this->_random, generate()).WillByDefault(Return("yolo random string"));

        this->_transport = std::make_shared<NiceMock<TransportMock>>();

        this->_factory = std::make_shared<NiceMock<TransportFactoryMock>>();
        ON_CALL(*this->_factory, create("http://yolo", _)).WillByDefault(Return(this->_transport));

        this->_conf = std::make_shared<NiceMock<JanusConfMock>>();
        ON_CALL(*this->_conf, url()).WillByDefault(Return("http://yolo"));
        ON_CALL(*this->_conf, plugin()).WillByDefault(Return("my yolo plugin"));

        this->_delegate = std::make_shared<NiceMock<ProtocolDelegateMock>>();

        this->_plugin = std::make_shared<NiceMock<PluginMock>>();

        this->_platform = std::make_shared<NiceMock<PlatformMock>>();
        ON_CALL(*this->_platform, plugin("my yolo plugin", _)).WillByDefault(Return(this->_plugin));
      }

      std::shared_ptr<TransportFactoryMock> _factory;
      std::shared_ptr<TransportMock> _transport;
      std::shared_ptr<NiceMock<JanusConfMock>> _conf;
      std::shared_ptr<NiceMock<ProtocolDelegateMock>> _delegate;
      std::shared_ptr<NiceMock<PlatformMock>> _platform;
      std::shared_ptr<NiceMock<RandomMock>> _random;
      std::shared_ptr<NiceMock<PluginMock>> _plugin;
  };

  TEST_F(JanusApiTest, shouldCreateANewSessionOnInit) {
    auto api = std::make_shared<JanusApi>(this->_random, this->_factory);
    EXPECT_CALL(*this->_factory, create("http://yolo", _)).Times(1);
    EXPECT_CALL(*this->_transport, send(IsJanusMessage("create"), BundleHasString("plugin", "my yolo plugin"))).Times(1);

    api->init(this->_conf, this->_platform, this->_delegate);
  }

  TEST_F(JanusApiTest, shouldAddTheCommandNameToPayload) {
    auto api = std::make_shared<JanusApi>(this->_random, this->_factory);
    auto bundle = Bundle::create();
    api->dispatch("yolo", bundle);

    EXPECT_EQ(bundle->getString("command", "INVALID"), "yolo");
  }

  TEST_F(JanusApiTest, shouldAttachOnSessionIdOnSuccess) {
    auto api = std::make_shared<JanusApi>(this->_random, this->_factory);
    api->init(this->_conf, this->_platform, this->_delegate);

    EXPECT_CALL(*this->_transport, sessionId("276911837174840")).Times(1);

    auto bundle = Bundle::create();
    bundle->setString("command", "create");
    bundle->setString("plugin", "my-plugin");

    EXPECT_CALL(*this->_transport, send(IsJanusMessage("attach"), bundle)).Times(1);

    nlohmann::json message = {
      { "janus", "success" },
      { "data", { { "id", 276911837174840 } } }
    };

    api->onMessage(message, bundle);
  }

  TEST_F(JanusApiTest, shouldCallTheOnReadyEventOnAttachSuccess) {
    EXPECT_CALL(*this->_delegate, onReady());

    auto api = std::make_shared<JanusApi>(this->_random, this->_factory);
    api->init(this->_conf, this->_platform, this->_delegate);

    auto bundle = Bundle::create();
    bundle->setString("command", "attach");
    bundle->setString("plugin", "my-plugin");

    nlohmann::json message = {
      { "janus", "success" },
      { "data", { { "id", 276911837174840 } } }
    };

    api->onMessage(message, bundle);
  }


  TEST_F(JanusApiTest, shouldSendADestroyOnClose) {
    auto api = std::make_shared<JanusApi>(this->_random, this->_factory);
    api->init(this->_conf, this->_platform, this->_delegate);

    EXPECT_CALL(*this->_transport, send(IsJanusMessage("destroy"), _)).Times(1);

    auto bundle = Bundle::create();
    bundle->setString("command", "attach");
    bundle->setString("plugin", "my-plugin");

    nlohmann::json message = {
      { "janus", "success" },
      { "data", { { "id", 276911837174840 } } }
    };

    api->onMessage(message, bundle);

    api->close();
    api->close();
  }

  TEST_F(JanusApiTest, shouldCloseTransportAndTriggerOnCloseOnDestroyEvent) {
    EXPECT_CALL(*this->_delegate, onClose()).Times(1);
    EXPECT_CALL(*this->_transport, close()).Times(1);

    auto api = std::make_shared<JanusApi>(this->_random, this->_factory);
    api->init(this->_conf, this->_platform, this->_delegate);

    auto bundle = Bundle::create();
    bundle->setString("command", "destroy");

    nlohmann::json message = {
      { "janus", "success" }
    };

    api->onMessage(message, bundle);
  }

  TEST_F(JanusApiTest, shouldHandleTheHangupEvent) {
    EXPECT_CALL(*this->_delegate, onHangup("my yolo reason")).Times(1);
    EXPECT_CALL(*this->_plugin, onHangup("my yolo reason")).Times(1);

    nlohmann::json message = {
      { "janus", "hangup" },
      { "reason", "my yolo reason" }
    };

    auto api = std::make_shared<JanusApi>(this->_random, this->_factory);
    api->init(this->_conf, this->_platform, this->_delegate);

    auto bundle = Bundle::create();
    bundle->setString("command", "attach");
    bundle->setString("plugin", "my yolo plugin");
    nlohmann::json attachMessage = {
      { "janus", "success" },
      { "data", { { "id", 276911837174840 } } }
    };
    api->onMessage(attachMessage, bundle);

    api->onMessage(message, Bundle::create());
  }

  TEST_F(JanusApiTest, shouldSendATrickleMessageOnIceCandidate) {
    auto api = std::make_shared<JanusApi>(this->_random, this->_factory);
    api->init(this->_conf, this->_platform, this->_delegate);

    nlohmann::json trickle = {
      { "janus", "trickle" },
      { "transaction", "yolo random string" },
      { "handle_id", 276911837174840 },
      { "candidate", { { "sdpMid", "yolo" }, { "sdpMLineIndex", 69 }, { "candidate", "my yolo candidate" } } }
    };

    {
      InSequence sequence;

      EXPECT_CALL(*this->_transport, send(IsJsonEq(trickle), BundleHasString("command", "trickle"))).Times(1);
      EXPECT_CALL(*this->_transport, send(_, BundleHasString("command", "destroy"))).Times(1);
    }

    auto bundle = Bundle::create();
    bundle->setString("command", "attach");
    nlohmann::json message = {
      { "janus", "success" },
      { "data", { { "id", 276911837174840 } } }
    };
    api->onMessage(message, bundle);

    api->onIceCandidate("yolo", 69, "my yolo candidate", bundle);
  }

  TEST_F(JanusApiTest, shouldSendATrickleCompletedMessageOnIceCompleted) {
    auto api = std::make_shared<JanusApi>(this->_random, this->_factory);
    api->init(this->_conf, this->_platform, this->_delegate);

    nlohmann::json trickle = {
      { "janus", "trickle" },
      { "transaction", "yolo random string" },
      { "handle_id", 276911837174840 },
      { "candidate", { { "completed", true } } }
    };

    {
      InSequence sequence;

      EXPECT_CALL(*this->_transport, send(IsJsonEq(trickle), BundleHasString("command", "trickle_completed"))).Times(1);
      EXPECT_CALL(*this->_transport, send(_, BundleHasString("command", "destroy"))).Times(1);
    }

    auto bundle = Bundle::create();
    bundle->setString("command", "attach");
    nlohmann::json message = {
      { "janus", "success" },
      { "data", { { "id", 276911837174840 } } }
    };
    api->onMessage(message, bundle);

    api->onIceCompleted(bundle);
  }

  TEST_F(JanusApiTest, shouldDelegateSdpEventsToPlugins) {
    auto api = std::make_shared<JanusApi>(this->_random, this->_factory);
    api->init(this->_conf, this->_platform, this->_delegate);

    auto context = Bundle::create();

    EXPECT_CALL(*this->_plugin, onOffer("the Offer", context)).Times(1);
    EXPECT_CALL(*this->_plugin, onAnswer("the Answer", context)).Times(1);

    auto bundle = Bundle::create();
    bundle->setString("command", "attach");
    bundle->setString("plugin", "my yolo plugin");

    nlohmann::json message = {
      { "janus", "success" },
      { "data", { { "id", 276911837174840 } } }
    };

    api->onMessage(message, bundle);
    api->onOffer("the Offer", context);
    api->onAnswer("the Answer", context);
  }

  TEST_F(JanusApiTest, shouldSendAPluginMessageOnCommandResultFired) {
    auto api = std::make_shared<JanusApi>(this->_random, this->_factory);
    api->init(this->_conf, this->_platform, this->_delegate);

    nlohmann::json message = {
      { "janus", "message" },
      { "transaction", "yolo random string" },
      { "handle_id", 276911837174840 },
      { "plugin_data", 42069 }
    };

    auto bundle = Bundle::create();

    {
      InSequence sequence;

      EXPECT_CALL(*this->_transport, send(IsJsonEq(message), bundle)).Times(1);
      EXPECT_CALL(*this->_transport, send(_, BundleHasString("command", "destroy"))).Times(1);
    }

    auto attachBundle = Bundle::create();
    attachBundle->setString("command", "attach");
    nlohmann::json attachMessage = {
      { "janus", "success" },
      { "data", { { "id", 276911837174840 } } }
    };
    api->onMessage(attachMessage, attachBundle);

    api->onCommandResult(message, bundle);

  }

  TEST_F(JanusApiTest, shouldOverrideHandleIdForPluginMessages) {
    auto api = std::make_shared<JanusApi>(this->_random, this->_factory);
    api->init(this->_conf, this->_platform, this->_delegate);

    nlohmann::json message = {
      { "janus", "message" },
      { "transaction", "yolo random string" },
      { "handle_id", 42069 },
      { "plugin_data", 42069 }
    };

    auto bundle = Bundle::create();

    {
      InSequence sequence;

      EXPECT_CALL(*this->_transport, send(IsJsonEq(message), bundle)).Times(1);
      EXPECT_CALL(*this->_transport, send(_, BundleHasString("command", "destroy"))).Times(1);
    }

    auto attachBundle = Bundle::create();
    attachBundle->setString("command", "attach");
    nlohmann::json attachMessage = {
      { "janus", "success" },
      { "data", { { "id", 276911837174840 } } }
    };
    api->onMessage(attachMessage, attachBundle);

    bundle->setInt("subscriberId", 42069);
    api->onCommandResult(message, bundle);

  }

  TEST_F(JanusApiTest, shouldDelegateToPluginCustomCommands) {
    auto api = std::make_shared<JanusApi>(this->_random, this->_factory);
    api->init(this->_conf, this->_platform, this->_delegate);

    auto bundle = Bundle::create();

    EXPECT_CALL(*this->_plugin, command("custom command", bundle)).Times(1);

    auto attachBundle = Bundle::create();
    attachBundle->setString("command", "attach");
    attachBundle->setString("plugin", "my yolo plugin");
    nlohmann::json attachMessage = {
      { "janus", "success" },
      { "data", { { "id", 276911837174840 } } }
    };
    api->onMessage(attachMessage, attachBundle);

    api->dispatch("custom command", bundle);
  }

  TEST_F(JanusApiTest, shouldDelegateEventsToPlugin) {
    auto api = std::make_shared<JanusApi>(this->_random, this->_factory);
    api->init(this->_conf, this->_platform, this->_delegate);

    auto bundle = Bundle::create();

    EXPECT_CALL(*this->_plugin, onEvent(IsEvent("custom", "custom event"), bundle)).Times(1);

    auto attachBundle = Bundle::create();
    attachBundle->setString("command", "attach");
    attachBundle->setString("plugin", "my yolo plugin");
    nlohmann::json attachMessage = {
      { "janus", "success" },
      { "data", { { "id", 276911837174840 } } }
    };
    api->onMessage(attachMessage, attachBundle);

    nlohmann::json message = {
      { "janus", "event" },
      { "plugindata", {
        { "data", {
          { "custom", "custom event" }
        } } }
      }
    }; 

    api->onMessage(message, bundle);
  }

  TEST_F(JanusApiTest, shouldHandleAJsepEvent) {
    auto api = std::make_shared<JanusApi>(this->_random, this->_factory);
    api->init(this->_conf, this->_platform, this->_delegate);

    auto bundle = Bundle::create();

    EXPECT_CALL(*this->_plugin, onEvent(HasJsep(SdpType::OFFER), bundle)).Times(1);

    auto attachBundle = Bundle::create();
    attachBundle->setString("command", "attach");
    attachBundle->setString("plugin", "my yolo plugin");
    nlohmann::json attachMessage = {
      { "janus", "success" },
      { "data", { { "id", 276911837174840 } } }
    };
    api->onMessage(attachMessage, attachBundle);

    nlohmann::json message = {
      { "janus", "event" },
      { "jsep", { { "type", "offer" }, { "sdp", "the sdp" } } },
      { "plugindata", {
        { "data", {
          { "custom", "custom event" }
        } } }
      }
    }; 

    api->onMessage(message, bundle);
  }

  TEST_F(JanusApiTest, shouldSendAnHangupMessageOnHangup) {
    auto api = std::make_shared<JanusApi>(this->_random, this->_factory);
    api->init(this->_conf, this->_platform, this->_delegate);

    {
      InSequence sequence;

      EXPECT_CALL(*this->_transport, send(IsJanusMessage("hangup"), BundleHasString("command", "hangup"))).Times(1);
      EXPECT_CALL(*this->_transport, send(_, BundleHasString("command", "destroy"))).Times(1);
    }

    auto attachBundle = Bundle::create();
    attachBundle->setString("command", "attach");
    attachBundle->setString("plugin", "my yolo plugin");
    nlohmann::json attachMessage = {
      { "janus", "success" },
      { "data", { { "id", 276911837174840 } } }
    };
    api->onMessage(attachMessage, attachBundle);

    api->hangup();
  }

  TEST_F(JanusApiTest, shouldDelegateAllTheOtherEvents) {
    auto api = std::make_shared<JanusApi>(this->_random, this->_factory);
    api->init(this->_conf, this->_platform, this->_delegate);

    auto bundle = Bundle::create();

    EXPECT_CALL(*this->_delegate, onEvent(IsEvent("janus", "custom"), bundle));

    nlohmann::json custom = {
      { "janus", "custom" }
    };

    api->onMessage(custom, bundle);
  }

  TEST_F(JanusApiTest, shouldDelegateTheErrorEvent) {
    auto api = std::make_shared<JanusApi>(this->_random, this->_factory);
    api->init(this->_conf, this->_platform, this->_delegate);

    auto bundle = Bundle::create();

    EXPECT_CALL(*this->_delegate, onError(IsError(69, "you only live once"), bundle));

    nlohmann::json error = {
      { "janus", "error" },
      { "error", {
        { "code", 69 },
        { "reason", "you only live once" }
      } }
    };

    api->onMessage(error, bundle);
  }

  TEST_F(JanusApiTest, shouldDelegatePluginEvents) {
    auto event = std::make_shared<JanusEventImpl>(nlohmann::json::object());
    auto context = Bundle::create();

    EXPECT_CALL(*this->_delegate, onEvent(Eq(event), Eq(context))).Times(1);

    auto api = std::make_shared<JanusApi>(this->_random, this->_factory);
    api->init(this->_conf, this->_platform, this->_delegate);
    api->onPluginEvent(event, context);
  }

  TEST_F(JanusApiTest, shouldDelegateSlaveAttachEvents) {
    auto api = std::make_shared<JanusApi>(this->_random, this->_factory);
    api->init(this->_conf, this->_platform, this->_delegate);

    auto attachBundle = Bundle::create();
    attachBundle->setString("command", "attach");
    attachBundle->setString("plugin", "my yolo plugin");
    nlohmann::json attachMessage = {
      { "janus", "success" },
      { "data", { { "id", 276911837174840 } } }
    };
    api->onMessage(attachMessage, attachBundle);

    auto bundle = Bundle::create();
    bundle->setString("command", "attach");
    bundle->setString("plugin", "my slave yolo plugin");
    nlohmann::json message = {
      { "janus", "success" },
      { "data", { { "id", 276911837174840 } } }
    };

    EXPECT_CALL(*this->_plugin, onEvent(IsEvent("janus", "success"), bundle)).Times(1);

    api->onMessage(message, bundle);
  }


}
