#include "janus/plugins/janus_plugin_echotest.h"

#include "janus/janus_commands.hpp"
#include "janus/constraints_builder.hpp"
#include "janus/constraints.hpp"
#include "janus/sdp_type.hpp"

namespace Janus {

  namespace Messages {

    nlohmann::json update(bool audio, bool video) {
      return {
        { "body", { { "audio", audio }, { "video", video } } }
      };
    }

    nlohmann::json call(const std::string& sdp, bool audio, bool video) {
      auto base = update(audio, video);
      base["jsep"] = { { "type", "offer" }, { "sdp", sdp } };

      return base;
    }

  }

  void JanusPluginEchotest::command(const std::string& command, const std::shared_ptr<Bundle>& payload) {
    if(command == JanusCommands::CALL) {
      this->_peer = this->_peerFactory->create(this->_handleId, this->_owner);
      auto constraints = payload->getConstraints();

      constraints.sdp.send_audio = constraints.sdp.receive_audio = payload->getBool("audio", true);
      constraints.sdp.send_video = constraints.sdp.receive_video = payload->getBool("video", true);
      constraints.sdp.datachannel = payload->getBool("datachannel", true);

      this->_peer->createOffer(constraints, payload);

      return;
    }

    if(command == JanusCommands::UPDATE) {
      auto msg = Messages::update(payload->getBool("audio", true), payload->getBool("video", true));
      this->_delegate->onCommandResult(msg, payload);

      return;
    }
  }

  void JanusPluginEchotest::onEvent(const std::shared_ptr<JanusEvent>& event, const std::shared_ptr<Bundle>& context) {
    auto jsep = event->jsep();

    if(jsep != nullptr) {
      this->_peer->setRemoteDescription(jsep->type(), jsep->sdp());

      return;
    }

    this->_delegate->onPluginEvent(event, context);
  }

  void JanusPluginEchotest::onOffer(const std::string& sdp, const std::shared_ptr<Bundle>& context) {
    this->_peer->setLocalDescription(SdpType::OFFER, sdp);

    auto msg = Messages::call(sdp, context->getBool("audio", true), context->getBool("video", true));
    this->_delegate->onCommandResult(msg, context);
  }

  JanusPluginEchotestFactory::JanusPluginEchotestFactory(const std::shared_ptr<PluginCommandDelegate>& delegate, const std::shared_ptr<PeerFactory>& peerFactory) {
    this->_peerFactory = peerFactory;
    this->_delegate = delegate;
  }

  std::shared_ptr<Plugin> JanusPluginEchotestFactory::create(int64_t handleId, const std::shared_ptr<Protocol>& owner) {
    auto plugin = std::make_shared<JanusPluginEchotest>(handleId, this->_delegate, this->_peerFactory, owner);

    return plugin;
  }

}
