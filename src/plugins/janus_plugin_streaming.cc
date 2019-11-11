#include "janus/plugins/janus_plugin_streaming.h"

#include "janus/janus_commands.hpp"
#include "janus/constraints.hpp"

namespace Janus {

  namespace Messages {

    nlohmann::json request(const std::string& request, const std::string& sdp="") {
      nlohmann::json msg = {
        { "body", { { "request", request } } }
      };

      if(sdp != "") {
        msg["jsep"] = { { "type", "answer" }, { "sdp", sdp } };
      }

      return msg;
    }

    nlohmann::json watch(int64_t id, bool offerAudio, bool offerVideo, bool offerData) {
      return {
        { "body", {
          { "request", "watch" },
          { "id", id },
          { "offer_audio", offerAudio },
          { "offer_video", offerVideo },
          { "offer_data", offerData }
        } }
      };
    }

  }

  void JanusPluginStreaming::command(const std::string& command, const std::shared_ptr<Bundle>& payload) {
    if(command == JanusCommands::LIST) {
      auto msg = Messages::request("list");
      this->_delegate->onCommandResult(msg, payload);

      return;
    }

    if(command == JanusCommands::WATCH) {
      this->_session = payload;

      auto id = payload->getInt("id", -1);
      auto offerAudio = payload->getBool("offer_audio", true);
      auto offerVideo = payload->getBool("offer_video", true);
      auto offerData = payload->getBool("offer_data", true);

      auto msg = Messages::watch(id, offerAudio, offerVideo, offerData);
      this->_delegate->onCommandResult(msg, payload);

      return;
    }

    if(command == JanusCommands::START) {
      auto msg = Messages::request("start");
      this->_delegate->onCommandResult(msg, payload);

      return;
    }

    if(command == JanusCommands::STOP) {
      auto msg = Messages::request("stop");
      this->_delegate->onCommandResult(msg, payload);

      return;
    }

    if(command == JanusCommands::PAUSE) {
      auto msg = Messages::request("pause");
      this->_delegate->onCommandResult(msg, payload);

      return;
    }
  }

  void JanusPluginStreaming::onEvent(const std::shared_ptr<JanusEvent>& event, const std::shared_ptr<Bundle>& context) {
    auto jsep = event->jsep();

    if(jsep != nullptr) {
      this->_peer = this->_peerFactory->create(this->_handleId, this->_owner);
      this->_peer->setRemoteDescription(jsep->type(), jsep->sdp());

      auto constraints = this->_session->getConstraints();
      constraints.sdp.send_audio = false;
      constraints.sdp.send_video = false;
      constraints.sdp.receive_audio = this->_session->getBool("offer_audio", true);
      constraints.sdp.receive_video = this->_session->getBool("offer_video", true);
      constraints.sdp.datachannel = this->_session->getBool("offer_data", true);

      this->_peer->createAnswer(constraints, context);

      return;
    }

    this->_delegate->onPluginEvent(event, context);
  }

  void JanusPluginStreaming::onAnswer(const std::string& sdp, const std::shared_ptr<Bundle>& context) {
    this->_peer->setLocalDescription(SdpType::ANSWER, sdp);

    auto msg = Messages::request("start", sdp);
    this->_delegate->onCommandResult(msg, this->_session);
  }

  JanusPluginStreamingFactory::JanusPluginStreamingFactory(const std::shared_ptr<PluginCommandDelegate>& delegate, const std::shared_ptr<PeerFactory>& peerFactory) {
    this->_peerFactory = peerFactory;
    this->_delegate = delegate;
  }

  std::shared_ptr<Plugin> JanusPluginStreamingFactory::create(int64_t handleId, const std::shared_ptr<Protocol>& owner) {
    auto plugin = std::make_shared<JanusPluginStreaming>(handleId, this->_delegate, this->_peerFactory, owner);

    return plugin;
  }

}
