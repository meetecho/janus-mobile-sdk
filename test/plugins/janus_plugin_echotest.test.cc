#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "janus/plugins/janus_plugin_echotest.h"

#include "janus/janus_commands.hpp"
#include "janus/bundle.hpp"
#include "janus/constraints_builder.hpp"
#include "janus/sdp_type.hpp"

#include "mocks/peer_factory.h"
#include "mocks/protocol.h"
#include "mocks/peer.h"
#include "mocks/plugin_command_delegate.h"
#include "mocks/matchers.h"

using testing::NiceMock;
using testing::HasConstraints;
using testing::IsJsonEq;
using testing::Return;
using testing::Eq;
using testing::InSequence;

namespace Janus {

  class JanusPluginEchotestTest : public testing::Test {
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

  TEST_F(JanusPluginEchotestTest, shouldCreateAnOfferOnCallCommand) {
    auto bundle = Bundle::create();

    auto builder = ConstraintsBuilder::create();
    auto constraints = builder->build();

    EXPECT_CALL(*this->_peer, createOffer(HasConstraints(constraints), bundle)).Times(1);
    auto plugin = std::make_shared<JanusPluginEchotest>(69, this->_delegate, this->_peerFactory, this->_owner);

    plugin->command(JanusCommands::CALL, bundle);
  }

  TEST_F(JanusPluginEchotestTest, shouldSetTheConstraints) {
    auto bundle = Bundle::create();
    bundle->setBool("audio", false);
    bundle->setBool("video", false);
    bundle->setBool("datachannel", false);

    auto builder = ConstraintsBuilder::create();
    auto constraints = builder->none()->build();

    EXPECT_CALL(*this->_peer, createOffer(HasConstraints(constraints), bundle)).Times(1);
    auto plugin = std::make_shared<JanusPluginEchotest>(69, this->_delegate, this->_peerFactory, this->_owner);

    plugin->command(JanusCommands::CALL, bundle);
  }

  TEST_F(JanusPluginEchotestTest, shouldUpdateTheCurrentSession) {
    nlohmann::json msg = {
      { "body", { { "audio", false }, { "video", false } } }
    };

    auto updates = Bundle::create();
    updates->setBool("audio", false);
    updates->setBool("video", false);

    EXPECT_CALL(*this->_delegate, onCommandResult(IsJsonEq(msg), updates));

    auto plugin = std::make_shared<JanusPluginEchotest>(69, this->_delegate, this->_peerFactory, this->_owner);

    auto bundle = Bundle::create();
    plugin->command(JanusCommands::CALL, bundle);

    plugin->command(JanusCommands::UPDATE, updates);
  }


  TEST_F(JanusPluginEchotestTest, shouldSendAJsepMessageOnOffer) {
    nlohmann::json msg = {
      { "body", { { "audio", true }, { "video", true } } },
      { "jsep", { { "type", "offer" }, { "sdp", "the sdp" } } }
    };

    auto context = Bundle::create();

    EXPECT_CALL(*this->_delegate, onCommandResult(IsJsonEq(msg), context));
    auto plugin = std::make_shared<JanusPluginEchotest>(69, this->_delegate, this->_peerFactory, this->_owner);

    auto bundle = Bundle::create();
    plugin->command(JanusCommands::CALL, bundle);

    plugin->onOffer("the sdp", context);
  }

  TEST_F(JanusPluginEchotestTest, shouldSetTheOfferAsLocalDescription) {
    EXPECT_CALL(*this->_peer, setLocalDescription(SdpType::OFFER, "the sdp"));

    auto plugin = std::make_shared<JanusPluginEchotest>(69, this->_delegate, this->_peerFactory, this->_owner);

    auto bundle = Bundle::create();
    plugin->command(JanusCommands::CALL, bundle);

    auto context = Bundle::create();
    plugin->onOffer("the sdp", context);
  }

  TEST_F(JanusPluginEchotestTest, shouldSetTheRemoteDescriptionOnJsepEvent) {
    EXPECT_CALL(*this->_peer, setRemoteDescription(SdpType::ANSWER, "the sdp"));

    nlohmann::json data = nlohmann::json::object();
    nlohmann::json jsep = {
      { "type", "answer" },
      { "sdp", "the sdp" }
    };

    auto event = std::make_shared<JanusEventImpl>(69, data, jsep);

    auto bundle = Bundle::create();

    auto plugin = std::make_shared<JanusPluginEchotest>(69, this->_delegate, this->_peerFactory, this->_owner);

    plugin->command(JanusCommands::CALL, bundle);

    plugin->onEvent(event, bundle);
  }

  TEST_F(JanusPluginEchotestTest, shouldCloseThePeerOnHangup) {
    EXPECT_CALL(*this->_peer, close()).Times(1);
    auto plugin = std::make_shared<JanusPluginEchotest>(69, this->_delegate, this->_peerFactory, this->_owner);

    auto bundle = Bundle::create();
    plugin->command(JanusCommands::CALL, bundle);
    plugin->onHangup("my reason");
  }

  TEST_F(JanusPluginEchotestTest, shouldAvoidSegFaultOnClose) {
    auto plugin = std::make_shared<JanusPluginEchotest>(69, this->_delegate, this->_peerFactory, this->_owner);
    plugin->onClose();
  }

  TEST_F(JanusPluginEchotestTest, shouldCleanupThePeerOnClose) {
    EXPECT_CALL(*this->_peer, close()).Times(1);
    auto plugin = std::make_shared<JanusPluginEchotest>(69, this->_delegate, this->_peerFactory, this->_owner);

    auto bundle = Bundle::create();
    plugin->command(JanusCommands::CALL, bundle);
    plugin->onClose();
  }

  TEST_F(JanusPluginEchotestTest, shouldDelegateUnhandledEvents) {
    auto context = Bundle::create();
    auto event = std::make_shared<JanusEventImpl>(69, nlohmann::json::object());
    EXPECT_CALL(*this->_delegate, onPluginEvent(Eq(event), Eq(context)));

    auto plugin = std::make_shared<JanusPluginEchotest>(69, this->_delegate, this->_peerFactory, this->_owner);
    plugin->onEvent(event, context);
  }

  class JanusPluginEchotestFactoryTest : public testing::Test {
  };

  TEST_F(JanusPluginEchotestFactoryTest, shouldCreateANewEchotestPlugin) {
    auto peerFactory = std::make_shared<NiceMock<PeerFactoryMock>>();
    auto owner = std::make_shared<NiceMock<ProtocolMock>>();
    auto delegate = std::make_shared<NiceMock<PluginCommandDelegateMock>>();

    auto factory = std::make_shared<JanusPluginEchotestFactory>(delegate, peerFactory);
    EXPECT_NE(factory->create(69, owner), nullptr);
  }

}
