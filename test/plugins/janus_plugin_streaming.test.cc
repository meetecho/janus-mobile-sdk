#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "janus/plugins/janus_plugin_streaming.h"

#include "janus/janus_commands.hpp"
#include "janus/constraints_builder.hpp" 

#include "mocks/peer_factory.h"
#include "mocks/protocol.h"
#include "mocks/peer.h"
#include "mocks/plugin_command_delegate.h"
#include "mocks/matchers.h"

using testing::NiceMock;
using testing::IsJsonEq;
using testing::HasConstraints;
using testing::InSequence;
using testing::Eq;

namespace Janus {

  class JanusPluginStreamingTest : public testing::Test {
    protected:
      void SetUp() override {
        this->_delegate = std::make_shared<NiceMock<PluginCommandDelegateMock>>();

        this->_peer = std::make_shared<NiceMock<PeerMock>>();
        this->_owner = std::make_shared<NiceMock<ProtocolMock>>();

        this->_peerFactory = std::make_shared<NiceMock<PeerFactoryMock>>();
        ON_CALL(*this->_peerFactory, create(69, Eq(this->_owner))).WillByDefault(Return(this->_peer));
      }

      std::shared_ptr<NiceMock<PeerMock>> _peer;
      std::shared_ptr<NiceMock<PluginCommandDelegateMock>> _delegate;
      std::shared_ptr<NiceMock<ProtocolMock>> _owner;
      std::shared_ptr<NiceMock<PeerFactoryMock>> _peerFactory;
  };

  TEST_F(JanusPluginStreamingTest, shouldSendAListMessage) {
    nlohmann::json msg = {
      { "body", { { "request", "list" } } }
    };

    auto bundle = Bundle::create();

    EXPECT_CALL(*this->_delegate, onCommandResult(IsJsonEq(msg), bundle));
    auto plugin = std::make_shared<JanusPluginStreaming>(69, this->_delegate, this->_peerFactory, this->_owner);
    plugin->command(JanusCommands::LIST, bundle);
  }

  TEST_F(JanusPluginStreamingTest, shouldSendAWatchMessage) {
    nlohmann::json msg = {
      { "body", {
        { "request", "watch" },
        { "id", 42069 },
        { "offer_audio", true },
        { "offer_video", true },
        { "offer_data", true }
      } }
    };

    auto bundle = Bundle::create();
    bundle->setInt("id", 42069);

    EXPECT_CALL(*this->_delegate, onCommandResult(IsJsonEq(msg), bundle));
    auto plugin = std::make_shared<JanusPluginStreaming>(69, this->_delegate, this->_peerFactory, this->_owner);
    plugin->command(JanusCommands::WATCH, bundle);
  }

  TEST_F(JanusPluginStreamingTest, shouldSetTheRemoteDescriptionAndGenerateAnswerOnJsepEvent) {
    auto builder = ConstraintsBuilder::create();
    auto constraints = builder->none()->datachannel(true)->receive_audio(true)->receive_video(true)->build();

    auto unused = Bundle::create();

    EXPECT_CALL(*this->_peer, setRemoteDescription(SdpType::OFFER, "the sdp"));
    EXPECT_CALL(*this->_peer, createAnswer(HasConstraints(constraints), unused)).Times(1);

    nlohmann::json data = nlohmann::json::object();
    nlohmann::json jsep = {
      { "type", "offer" },
      { "sdp", "the sdp" }
    };

    auto event = std::make_shared<JanusEventImpl>(69, data, jsep);

    auto bundle = Bundle::create();
    bundle->setInt("id", 42069);

    auto plugin = std::make_shared<JanusPluginStreaming>(69, this->_delegate, this->_peerFactory, this->_owner);
    plugin->command(JanusCommands::WATCH, bundle);
    plugin->onEvent(event, unused);
  }

  TEST_F(JanusPluginStreamingTest, shouldSetTheLocalDescriptionAndSendTheAnswerToJanus) {
    nlohmann::json msg = {
      { "body", { { "request", "start" } } },
      { "jsep", { { "type", "answer" }, { "sdp", "the sdp" } } }
    };

    nlohmann::json watchMsg = {
      { "body", {
        { "request", "watch" },
        { "id", 42069 },
        { "offer_audio", true },
        { "offer_video", true },
        { "offer_data", true }
      } }
    };


    auto watchBundle = Bundle::create();
    watchBundle->setInt("id", 42069);

    {
      InSequence seq;
      EXPECT_CALL(*this->_delegate, onCommandResult(IsJsonEq(watchMsg), watchBundle));
      EXPECT_CALL(*this->_delegate, onCommandResult(IsJsonEq(msg), watchBundle));
    }

    EXPECT_CALL(*this->_peer, setLocalDescription(SdpType::ANSWER, "the sdp"));
    auto plugin = std::make_shared<JanusPluginStreaming>(69, this->_delegate, this->_peerFactory, this->_owner);

    plugin->command(JanusCommands::WATCH, watchBundle);

    nlohmann::json data = nlohmann::json::object();
    nlohmann::json jsep = {
      { "type", "offer" },
      { "sdp", "the sdp" }
    };
    auto event = std::make_shared<JanusEventImpl>(69, data, jsep);
    auto unused = Bundle::create();
    plugin->onEvent(event, unused);

    auto bundle = Bundle::create();
    plugin->onAnswer("the sdp", bundle);
  }

  TEST_F(JanusPluginStreamingTest, shouldSendStartCommand) {
    nlohmann::json msg = {
      { "body", { { "request", "start" } } }
    };
    auto bundle = Bundle::create();

    EXPECT_CALL(*this->_delegate, onCommandResult(IsJsonEq(msg), bundle));

    auto plugin = std::make_shared<JanusPluginStreaming>(69, this->_delegate, this->_peerFactory, this->_owner);
    plugin->command(JanusCommands::START, bundle);
  }

  TEST_F(JanusPluginStreamingTest, shouldSendStopCommand) {
    nlohmann::json msg = {
      { "body", { { "request", "stop" } } }
    };
    auto bundle = Bundle::create();

    EXPECT_CALL(*this->_delegate, onCommandResult(IsJsonEq(msg), bundle));

    auto plugin = std::make_shared<JanusPluginStreaming>(69, this->_delegate, this->_peerFactory, this->_owner);
    plugin->command(JanusCommands::STOP, bundle);
  }


  TEST_F(JanusPluginStreamingTest, shouldSendPauseCommand) {
    nlohmann::json msg = {
      { "body", { { "request", "pause" } } }
    };
    auto bundle = Bundle::create();

    EXPECT_CALL(*this->_delegate, onCommandResult(IsJsonEq(msg), bundle));

    auto plugin = std::make_shared<JanusPluginStreaming>(69, this->_delegate, this->_peerFactory, this->_owner);
    plugin->command(JanusCommands::PAUSE, bundle);
  }

  TEST_F(JanusPluginStreamingTest, shouldDelegateUnhandledEvents) {
    auto context = Bundle::create();
    auto event = std::make_shared<JanusEventImpl>(69, nlohmann::json::object());
    EXPECT_CALL(*this->_delegate, onPluginEvent(Eq(event), Eq(context)));

    auto plugin = std::make_shared<JanusPluginStreaming>(69, this->_delegate, this->_peerFactory, this->_owner);
    plugin->onEvent(event, context);
  }

  class JanusPluginStreamingFactoryTest : public testing::Test {
  };

  TEST_F(JanusPluginStreamingFactoryTest, shouldCreateANewStreamingPlugin) {
    auto peerFactory = std::make_shared<NiceMock<PeerFactoryMock>>();
    auto owner = std::make_shared<NiceMock<ProtocolMock>>();
    auto delegate = std::make_shared<NiceMock<PluginCommandDelegateMock>>();

    auto factory = std::make_shared<JanusPluginStreamingFactory>(delegate, peerFactory);
    EXPECT_NE(factory->create(69, owner), nullptr);
  }

}
