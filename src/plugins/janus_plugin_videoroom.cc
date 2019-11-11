#include "janus/plugins/janus_plugin_videoroom.h"

#include "janus/janus_commands.hpp"
#include "janus/constraints_builder_impl.h"
#include "janus/janus_p_types.hpp"

namespace Janus {

  namespace Messages {

    nlohmann::json start(const std::string& sdp) {
      return {
        { "body", {
          { "request", "start" }
        } },
        { "jsep", {
          { "type", "answer" },
          { "sdp", sdp }
        } }
      };
    }

    nlohmann::json list() {
      return {
        { "body", { { "request", "list" } } }
      };
    }

    nlohmann::json publish(const std::string& sdp, bool audio, bool video, bool data) {
      return {
        { "body", {
          { "request", "publish" },
          { "audio", audio },
          { "video", video },
          { "data", data }
        } },
        { "jsep", {
          { "type", "offer" },
          { "sdp", sdp }
        } }
      };
    }

    nlohmann::json listParticipants(int64_t room) {
      return {
        { "body", {
          { "request", "listparticipants" },
          { "room", room }
        } }
      };
    }

    nlohmann::json subscribe(int64_t room, int64_t feed, bool offer_audio, bool offer_video, bool offer_data) {
      return {
        { "body", {
          { "request", "join" },
          { "ptype", "subscriber" },
          { "room", room },
          { "feed", feed },
          { "offer_audio", offer_audio },
          { "offer_video", offer_video },
          { "offer_data", offer_data }
        } }
      };
    }


    nlohmann::json join(const std::string& ptype, int64_t room, const std::string& display, int64_t id, const std::string& token) {
      nlohmann::json msg = {
        { "body", {
          { "request", "join" },
          { "ptype", ptype },
          { "room", room }
        } }
      };

      if(display != "") {
        msg["body"]["display"] = display;
      }

      if(id != -1) {
        msg["body"]["id"] = id;
      }

      if(token != "") {
        msg["body"]["token"] = token;
      }

      return msg;
    }

  }

  void JanusPluginVideoroom::command(const std::string& command, const std::shared_ptr<Bundle>& payload) {

    if(command == JanusCommands::LIST) {
      auto msg = Messages::list();
      this->_delegate->onCommandResult(msg, payload);

      return;
    }

    if(command == JanusCommands::LISTPARTICIPANTS) {
      auto room = payload->getInt("room", -1);
      auto msg = Messages::listParticipants(room);
      this->_delegate->onCommandResult(msg, payload);

      return;
    }

    if(command == JanusCommands::JOIN) {
      auto room = payload->getInt("room", -1);
      auto display = payload->getString("display", "");
      auto id = payload->getInt("id", -1);
      auto token = payload->getString("token", "");
      auto ptype = payload->getString("ptype", JanusPTypes::PUBLISHER);

      auto msg = Messages::join(ptype, room, display, id, token);
      this->_delegate->onCommandResult(msg, payload);

      return;
    }

    if(command == JanusCommands::PUBLISH) {
      this->_peer = this->_peerFactory->create(this->_handleId, this->_owner);

      auto constraints = payload->getConstraints();

      constraints.sdp.receive_audio = false;
      constraints.sdp.send_audio = payload->getBool("audio", true);
      constraints.sdp.receive_video = false;
      constraints.sdp.send_video = payload->getBool("video", true);
      constraints.sdp.datachannel = payload->getBool("datachannel", true);

      this->_peer->createOffer(constraints, payload);

      return;
    }

    if(command == JanusCommands::SUBSCRIBE) {
      payload->setString("plugin", JanusPlugins::VIDEOROOM);
      this->_owner->dispatch(JanusCommands::ATTACH, payload);

      return;
    }

  }

  void JanusPluginVideoroom::onEvent(const std::shared_ptr<JanusEvent>& event, const std::shared_ptr<Bundle>& context) {
    auto data = event->data();
    auto jsep = event->jsep();

    if(data->getString("configured", "") == "ok" && jsep != nullptr) {
      this->_peer->setRemoteDescription(jsep->type(), jsep->sdp());

      return;
    }

    if(data->getString("janus", "") == "success" && context->getString("command", "") == "attach") {
      auto subscriberId = data->getObject("data")->getInt("id", -1);

      auto peer = this->_peerFactory->create(subscriberId, this->_owner);
      auto subscriber = std::make_shared<Subscriber>(peer, context);
      this->_subscribers[subscriberId] = subscriber;

      context->setInt("handleId", subscriberId);

      auto offer_audio = context->getBool("offer_audio", true);
      auto offer_video = context->getBool("offer_video", true);
      auto offer_data = context->getBool("offer_data", true);
      auto feed = context->getInt("feed", -1);
      auto room = context->getInt("room", -1);

      auto msg = Messages::subscribe(room, feed, offer_audio, offer_video, offer_data);
      this->_delegate->onCommandResult(msg, context);

      return;
    }

    if(data->getString("videoroom", "") == "attached" && jsep != nullptr) {
      auto subscriberId = event->sender();
      auto subscriber = this->_subscribers[subscriberId];

      auto peer = subscriber->peer;

      peer->setRemoteDescription(jsep->type(), jsep->sdp());

      auto subscriberContext = subscriber->context;

      auto constraints = subscriberContext->getConstraints();
      constraints.sdp.send_audio = false;
      constraints.sdp.send_video = false;
      constraints.sdp.receive_audio = subscriberContext->getBool("offer_audio", true);
      constraints.sdp.receive_video = subscriberContext->getBool("offer_video", true);
      constraints.sdp.datachannel = subscriberContext->getBool("offer_data", true);

      peer->createAnswer(constraints, subscriberContext);

      return;
    }

    this->_delegate->onPluginEvent(event, context);
  }

  void JanusPluginVideoroom::onOffer(const std::string& sdp, const std::shared_ptr<Bundle>& context) {
    this->_peer->setLocalDescription(SdpType::OFFER, sdp);

    auto audio = context->getBool("audio", true);
    auto video = context->getBool("video", true);
    auto data = context->getBool("data", true);

    auto msg = Messages::publish(sdp, audio, video, data);
    this->_delegate->onCommandResult(msg, context);
  }

  void JanusPluginVideoroom::onAnswer(const std::string& sdp, const std::shared_ptr<Bundle>& context) {
    auto subscriberId = context->getInt("handleId", -1);
    auto subscriber = this->_subscribers[subscriberId];

    auto peer = subscriber->peer;
    peer->setLocalDescription(SdpType::ANSWER, sdp);

    auto msg = Messages::start(sdp);
    this->_delegate->onCommandResult(msg, context);
  }

  JanusPluginVideoroomFactory::JanusPluginVideoroomFactory(const std::shared_ptr<PluginCommandDelegate>& delegate, const std::shared_ptr<PeerFactory>& peerFactory) {
    this->_peerFactory = peerFactory;
    this->_delegate = delegate;
  }

  std::shared_ptr<Plugin> JanusPluginVideoroomFactory::create(int64_t handleId, const std::shared_ptr<Protocol>& owner) {
    auto plugin = std::make_shared<JanusPluginVideoroom>(handleId, this->_delegate, this->_peerFactory, owner);

    return plugin;
  }

}
