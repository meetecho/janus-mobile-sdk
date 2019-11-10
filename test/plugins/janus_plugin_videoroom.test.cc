#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "janus/plugins/janus_plugin_videoroom.h"

#include "janus/janus_commands.hpp"
#include "janus/constraints_builder.hpp"

#include "mocks/peer_factory.h"
#include "mocks/protocol.h"
#include "mocks/plugin_command_delegate.h"
#include "mocks/peer.h"
#include "mocks/matchers.h"

using testing::NiceMock;
using testing::IsJsonEq;
using testing::Eq;
using testing::HasConstraints;
using testing::BundleHasString;
using testing::BundleHasInt;

namespace Janus {

  class JanusPluginVideoroomTest : public testing::Test {
    protected:
      void SetUp() override {
        this->_delegate = std::make_shared<NiceMock<PluginCommandDelegateMock>>();

        this->_peer = std::make_shared<NiceMock<PeerMock>>();
        this->_owner = std::make_shared<NiceMock<ProtocolMock>>();

        this->_peerFactory = std::make_shared<NiceMock<PeerFactoryMock>>();
        ON_CALL(*this->_peerFactory, create(-1, Eq(this->_owner))).WillByDefault(Return(this->_peer));
        ON_CALL(*this->_peerFactory, create(69420, Eq(this->_owner))).WillByDefault(Return(this->_peer));
      }

      std::shared_ptr<NiceMock<PeerMock>> _peer;
      std::shared_ptr<NiceMock<PluginCommandDelegateMock>> _delegate;
      std::shared_ptr<NiceMock<ProtocolMock>> _owner;
      std::shared_ptr<NiceMock<PeerFactoryMock>> _peerFactory;
  };

  TEST_F(JanusPluginVideoroomTest, shouldSendAListMessage) {
    nlohmann::json msg = {
      { "body", { { "request", "list" } } }
    };

    auto bundle = Bundle::create();

    EXPECT_CALL(*this->_delegate, onCommandResult(IsJsonEq(msg), bundle));
    auto plugin = std::make_shared<JanusPluginVideoroom>(this->_delegate, this->_peerFactory, this->_owner);
    plugin->command(JanusCommands::LIST, bundle);
  }

  TEST_F(JanusPluginVideoroomTest, shouldSendAListParticipantMessage) {
    nlohmann::json msg = {
      { "body", { { "request", "listparticipants" }, { "room", 42069 } } }
    };

    auto bundle = Bundle::create();
    bundle->setInt("room", 42069);

    EXPECT_CALL(*this->_delegate, onCommandResult(IsJsonEq(msg), bundle));
    auto plugin = std::make_shared<JanusPluginVideoroom>(this->_delegate, this->_peerFactory, this->_owner);
    plugin->command(JanusCommands::LISTPARTICIPANTS, bundle);
  }

  TEST_F(JanusPluginVideoroomTest, shouldSendAJoinMessage) {
    nlohmann::json msg = {
      { "body", {
        { "ptype", "publisher" },
        { "request", "join" },
        { "room", 42069 },
        { "display", "yolo" },
        { "id", 69420 },
        { "token", "my token" }
      } }
    };

    auto bundle = Bundle::create();
    bundle->setInt("room", 42069);
    bundle->setString("display", "yolo");
    bundle->setInt("id", 69420); 
    bundle->setString("token", "my token");

    EXPECT_CALL(*this->_delegate, onCommandResult(IsJsonEq(msg), bundle));
    auto plugin = std::make_shared<JanusPluginVideoroom>(this->_delegate, this->_peerFactory, this->_owner);
    plugin->command(JanusCommands::JOIN, bundle);
  }

  TEST_F(JanusPluginVideoroomTest, shouldSkipOptionalFieldsOnJoinMessage) {
    nlohmann::json msg = {
      { "body", {
        { "request", "join" },
        { "ptype", "publisher" },
        { "room", 42069 }
      } }
    };

    auto bundle = Bundle::create();
    bundle->setInt("room", 42069);

    EXPECT_CALL(*this->_delegate, onCommandResult(IsJsonEq(msg), bundle));
    auto plugin = std::make_shared<JanusPluginVideoroom>(this->_delegate, this->_peerFactory, this->_owner);
    plugin->command(JanusCommands::JOIN, bundle);
  }

  TEST_F(JanusPluginVideoroomTest, shouldDelegateUnhandledEvents) {
    auto context = Bundle::create();
    auto event = std::make_shared<JanusEventImpl>(nlohmann::json::object());
    EXPECT_CALL(*this->_delegate, onPluginEvent(Eq(event), Eq(context)));

    auto plugin = std::make_shared<JanusPluginVideoroom>(this->_delegate, this->_peerFactory, this->_owner);
    plugin->onEvent(event, context);
  }

  TEST_F(JanusPluginVideoroomTest, shouldCreateAnOfferOnPublish) {
    auto context = Bundle::create();

    auto builder = ConstraintsBuilder::create();
    auto constraints = builder->receive_audio(false)->receive_video(false)->build();

    EXPECT_CALL(*this->_peer, createOffer(HasConstraints(constraints), context)).Times(1);

    auto plugin = std::make_shared<JanusPluginVideoroom>(this->_delegate, this->_peerFactory, this->_owner);
    plugin->command(JanusCommands::PUBLISH, context);
  }

  TEST_F(JanusPluginVideoroomTest, shouldSetTheConstraints) {
    auto bundle = Bundle::create();
    bundle->setBool("audio", false);
    bundle->setBool("video", false);
    bundle->setBool("datachannel", false);

    auto builder = ConstraintsBuilder::create();
    auto constraints = builder->none()->build();

    EXPECT_CALL(*this->_peer, createOffer(HasConstraints(constraints), bundle)).Times(1);
    auto plugin = std::make_shared<JanusPluginVideoroom>(this->_delegate, this->_peerFactory, this->_owner);

    plugin->command(JanusCommands::PUBLISH, bundle);
  }

  TEST_F(JanusPluginVideoroomTest, shouldSendAJsepMessageOnOffer) {
    nlohmann::json msg = {
      { "body", { { "request", "publish" }, { "audio", true }, { "video", true }, { "data", true } } },
      { "jsep", { { "type", "offer" }, { "sdp", "the sdp" } } }
    };

    auto context = Bundle::create();

    EXPECT_CALL(*this->_peer, setLocalDescription(SdpType::OFFER, "the sdp"));
    EXPECT_CALL(*this->_delegate, onCommandResult(IsJsonEq(msg), context));
    auto plugin = std::make_shared<JanusPluginVideoroom>(this->_delegate, this->_peerFactory, this->_owner);

    plugin->command(JanusCommands::PUBLISH, context);

    plugin->onOffer("the sdp", context);
  }

  TEST_F(JanusPluginVideoroomTest, shouldSetTheRemoteDescriptionOnConfiguredEvent) {
    EXPECT_CALL(*this->_peer, setRemoteDescription(SdpType::ANSWER, "the sdp"));

    nlohmann::json data = {
      { "videoroom", "event" },
      { "configured", "ok" }
    };
    nlohmann::json jsep = {
      { "type", "answer" },
      { "sdp", "the sdp" }
    };

    auto event = std::make_shared<JanusEventImpl>(data, jsep);

    auto bundle = Bundle::create();

    auto plugin = std::make_shared<JanusPluginVideoroom>(this->_delegate, this->_peerFactory, this->_owner);

    plugin->command(JanusCommands::PUBLISH, bundle);

    plugin->onEvent(event, bundle);
  }

  TEST_F(JanusPluginVideoroomTest, shouldCallAttachCommandOnSubscribe) {
    EXPECT_CALL(*this->_owner, dispatch(JanusCommands::ATTACH, BundleHasString("plugin", JanusPlugins::VIDEOROOM)));

    auto bundle = Bundle::create();
    auto plugin = std::make_shared<JanusPluginVideoroom>(this->_delegate, this->_peerFactory, this->_owner);
    plugin->command(JanusCommands::SUBSCRIBE, bundle);
  }

  TEST_F(JanusPluginVideoroomTest, shouldAddSubscriberIdToContextOnAttach) {
    nlohmann::json msg = {
      { "body", { { "request", "join" }, { "ptype", "subscriber" }, { "room", 69 }, { "feed", 420 }, { "offer_audio", true }, { "offer_video", true }, { "offer_data", true } } }
    };

    auto bundle = Bundle::create();
    bundle->setString("command", "attach");
    bundle->setString("plugin", JanusPlugins::VIDEOROOM);
    bundle->setInt("feed", 420);
    bundle->setInt("room", 69);

    EXPECT_CALL(*this->_delegate, onCommandResult(IsJsonEq(msg), BundleHasInt("subscriberId", 69420)));

    auto plugin = std::make_shared<JanusPluginVideoroom>(this->_delegate, this->_peerFactory, this->_owner);

    nlohmann::json attachEvent = {
      { "janus", "success" },
      { "data", { { "id", 69420 } } }
    };
    auto evt = std::make_shared<JanusEventImpl>(attachEvent);
    plugin->onEvent(evt, bundle);
  }

  TEST_F(JanusPluginVideoroomTest, shouldSetTheRemoteDescriptionAndGenerateAnswerOnJsepEvent) {
    auto builder = ConstraintsBuilder::create();
    auto constraints = builder->none()->datachannel(true)->receive_audio(true)->receive_video(true)->build();

    auto bundle = Bundle::create();

    EXPECT_CALL(*this->_peer, setRemoteDescription(SdpType::OFFER, "the sdp"));
    EXPECT_CALL(*this->_peer, createAnswer(HasConstraints(constraints), bundle)).Times(1);

    nlohmann::json data = {
      { "videoroom", "attached" }
    };
    nlohmann::json jsep = {
      { "type", "offer" },
      { "sdp", "the sdp" }
    };

    auto event = std::make_shared<JanusEventImpl>(data, jsep);

    auto plugin = std::make_shared<JanusPluginVideoroom>(this->_delegate, this->_peerFactory, this->_owner);
    plugin->onEvent(event, bundle);
  }

  TEST_F(JanusPluginVideoroomTest, shouldSetTheLocalDescriptionAndSendTheAnswerToJanus) {
    auto bundle = Bundle::create();
    bundle->setInt("subscriberId", 69420);

    nlohmann::json msg = {
      { "body", {
        { "request", "start" }
      } },
      { "jsep", {
        { "type", "answer" },
        { "sdp", "the sdp" }
      } }
    };

    EXPECT_CALL(*this->_peer, setLocalDescription(SdpType::ANSWER, "the sdp"));
    EXPECT_CALL(*this->_delegate, onCommandResult(msg, bundle));

    nlohmann::json data = {
      { "videoroom", "attached" }
    };
    nlohmann::json jsep = {
      { "type", "offer" },
      { "sdp", "the sdp" }
    };

    auto event = std::make_shared<JanusEventImpl>(data, jsep);

    auto plugin = std::make_shared<JanusPluginVideoroom>(this->_delegate, this->_peerFactory, this->_owner);
    plugin->onEvent(event, bundle);

    plugin->onAnswer("the sdp", bundle);
  }


  class JanusPluginVideoroomFactoryTest : public testing::Test {
  };

  TEST_F(JanusPluginVideoroomFactoryTest, shouldCreateANewVideoroomPlugin) {
    auto peerFactory = std::make_shared<NiceMock<PeerFactoryMock>>();
    auto owner = std::make_shared<NiceMock<ProtocolMock>>();
    auto delegate = std::make_shared<NiceMock<PluginCommandDelegateMock>>();

    auto factory = std::make_shared<JanusPluginVideoroomFactory>(delegate, peerFactory);
    EXPECT_NE(factory->create(owner), nullptr);
  }

}
