#include "janus/janus_api.h"

#include "janus/bundle_impl.h"
#include "janus/janus_error.hpp"
#include "janus/janus_commands.hpp"

namespace Janus {

  /* Janus API message Factories */
  
  namespace Messages {

    nlohmann::json create(const std::string& transaction) {
      return {
        { "janus", JanusCommands::CREATE },
        { "transaction", transaction }
      };
    }

    nlohmann::json attach(const std::string& transaction, const std::string& plugin) {
      return {
        { "janus", JanusCommands::ATTACH },
        { "plugin", plugin },
        { "transaction", transaction }
      };
    }

    nlohmann::json destroy(const std::string& transaction) {
      return {
        { "janus", JanusCommands::DESTROY },
        { "transaction", transaction }
      };
    }

    nlohmann::json trickle(const std::string& transaction, int64_t handleId, const std::string& sdpMid, int32_t sdpMLineIndex, const std::string& candidate) {
      return {
        { "janus", JanusCommands::TRICKLE },
        { "transaction", transaction },
        { "handle_id", handleId },
        { "candidate", { { "sdpMid", sdpMid }, { "sdpMLineIndex", sdpMLineIndex }, { "candidate", candidate } } }
      };
    }

    nlohmann::json trickleCompleted(const std::string& transaction, int64_t handleId) {
      return {
        { "janus", JanusCommands::TRICKLE },
        { "transaction", transaction },
        { "handle_id", handleId },
        { "candidate", { { "completed", true } } }
      };
    }

    nlohmann::json message(const std::string& transaction, int64_t handleId, nlohmann::json body) {
      body["janus"] = "message";
      body["transaction"] = transaction;
      body["handle_id"] = handleId;

      return body;
    }

    nlohmann::json hangup(const std::string& transaction, int64_t handleId) {
      return {
        { "janus", JanusCommands::HANGUP },
        { "transaction", transaction },
        { "handle_id", handleId }
      };
    }

  }

  /* Janus API */

  JanusApi::JanusApi(const std::shared_ptr<Random>& random, const std::shared_ptr<TransportFactory>& transportFactory) {
    this->_transportFactory = transportFactory;
    this->_random = random;
  }

  JanusApi::~JanusApi() {
    this->close();
  }

  void JanusApi::init(const std::shared_ptr<JanusConf>& conf, const std::shared_ptr<Platform>& platform, const std::shared_ptr<ProtocolDelegate>& delegate) {
    this->readyState(ReadyState::INIT);

    this->_transport = this->_transportFactory->create(conf->url(), this->shared_from_this());
    this->_delegate = delegate;
    this->_platform = std::static_pointer_cast<PlatformImpl>(platform);

    auto bundle = Bundle::create();
    bundle->setString("plugin", conf->plugin());
    this->dispatch(JanusCommands::CREATE, bundle);
  }

  void JanusApi::dispatch(const std::string& command, const std::shared_ptr<Bundle>& payload) {
    payload->setString("command", command);
    auto transaction = this->_random->generate();
    auto handleId = this->handleId(payload);

    if(command == JanusCommands::CREATE) {
      auto msg = Messages::create(transaction);
      this->_transport->send(msg, payload);

      return;
    }

    if(command == JanusCommands::ATTACH) {
      auto plugin = payload->getString("plugin", "");
      this->_transport->send(Messages::attach(transaction, plugin), payload);

      return;
    }

    if(command == JanusCommands::DESTROY) {
      this->_transport->send(Messages::destroy(transaction), payload);

      return;
    }

    if(command == JanusCommands::HANGUP) {
      this->_transport->send(Messages::hangup(transaction, handleId), payload);

      return;
    }

    if(command == JanusCommands::TRICKLE) {
      auto sdpMid = payload->getString("sdpMid", "");
      auto sdpMLineIndex = payload->getInt("sdpMLineIndex", -1);
      auto candidate = payload->getString("candidate", "");

      auto msg = Messages::trickle(transaction, handleId, sdpMid, sdpMLineIndex, candidate);
      this->_transport->send(msg, payload);

      return;
    }

    if(command == JanusCommands::TRICKLE_COMPLETED) {
      auto msg = Messages::trickleCompleted(transaction, handleId);
      this->_transport->send(msg, payload);

      return;
    }

    if(this->_plugin != nullptr) {
      this->_plugin->command(command, payload);
    }
  }

  void JanusApi::hangup() {
    auto bundle = Bundle::create();
    this->dispatch(JanusCommands::HANGUP, bundle);
  }

  void JanusApi::close() {
    if(this->readyState() != ReadyState::READY) {
      return;
    }

    this->readyState(ReadyState::CLOSING);

    auto bundle = Bundle::create();
    this->dispatch(JanusCommands::DESTROY, bundle);
  }

  void JanusApi::onMessage(const nlohmann::json& message, const std::shared_ptr<Bundle>& context) {
    auto header = message.value("janus", "");
    auto str = message.dump();

    if(header == "error") {
      auto errorContent = message.value("error", nlohmann::json::object());
      auto code = errorContent.value("code", -1);
      auto reason = errorContent.value("reason", "");

      JanusError error(code, reason);
      this->_delegate->onError(error, context);

      return;
    }

    if(header == "success" && context->getString("command", "") == JanusCommands::CREATE) {
      auto id = message.value("data", nlohmann::json::object()).value("id", (int64_t) 0);
      auto idAsString = std::to_string(id);
      this->_transport->sessionId(idAsString);
      this->dispatch(JanusCommands::ATTACH, context);

      return;
    }

    if(header == "success" && context->getString("command", "") == JanusCommands::ATTACH && this->_plugin == nullptr) {
      this->_handleId = message.value("data", nlohmann::json::object()).value("id", (int64_t) 0);

      auto pluginId = context->getString("plugin", "");
      this->_plugin = this->_platform->plugin(pluginId, this->_handleId, this->shared_from_this());

      this->readyState(ReadyState::READY);
      this->_delegate->onReady();

      return;
    }

    if(header == "success" && context->getString("command", "") == JanusCommands::DESTROY) {
      this->_transport->close();
      this->readyState(ReadyState::CLOSED);
      this->_delegate->onClose();

      return;
    }

    if(header == "hangup") {
      auto reason = message.value("reason", "");

      this->_plugin->onHangup(reason);
      this->_delegate->onHangup(reason);

      return;
    }

    auto sender = message.value("sender", this->_handleId);

    if(header == "event") {
      auto data = message.value("plugindata", nlohmann::json::object()).value("data", nlohmann::json::object());
      auto jsep = message.value("jsep", nlohmann::json::object());

      std::shared_ptr<JanusEventImpl> evt;
      if(jsep.empty()) {
        evt = std::make_shared<JanusEventImpl>(sender, data);
      } else {
        evt = std::make_shared<JanusEventImpl>(sender, data, jsep);
      }
      this->_plugin->onEvent(evt, context);

      return;
    }

    auto evt = std::make_shared<JanusEventImpl>(sender, message);

    if(header == "success" && context->getString("command", "") == JanusCommands::ATTACH && this->_plugin != nullptr) {
      this->_plugin->onEvent(evt, context);

      return;
    }

    this->_delegate->onEvent(evt, context);
  }

  void JanusApi::onOffer(const std::string& sdp, const std::shared_ptr<Bundle>& context) {
    this->_plugin->onOffer(sdp, context);
  }

  void JanusApi::onAnswer(const std::string& sdp, const std::shared_ptr<Bundle>& context) {
    this->_plugin->onAnswer(sdp, context);
  }

  void JanusApi::onIceCandidate(const std::string& mid, int32_t index, const std::string& sdp, int64_t id) {
    auto bundle = Bundle::create();
    bundle->setString("sdpMid", mid);
    bundle->setInt("sdpMLineIndex", index);
    bundle->setString("candidate", sdp);
    bundle->setInt("handleId", id);

    this->dispatch(JanusCommands::TRICKLE, bundle);
  }

  void JanusApi::onIceCompleted(int64_t id) {
    auto bundle = Bundle::create();
    bundle->setInt("handleId", id);

    this->dispatch(JanusCommands::TRICKLE_COMPLETED, bundle);
  }

  ReadyState JanusApi::readyState() {
    std::lock_guard<std::mutex> lock(this->_readyStateMutex);

    return this->_readyState;
  }

  void JanusApi::readyState(ReadyState readyState) {
    std::lock_guard<std::mutex> lock(this->_readyStateMutex);
    this->_readyState = readyState;
  }

  int64_t JanusApi::handleId(const std::shared_ptr<Bundle>& context) {
    return context->getInt("handleId", this->_handleId);
  }

  void JanusApi::onCommandResult(const nlohmann::json& body, const std::shared_ptr<Bundle>& context) {
    auto transaction = this->_random->generate();
    auto handleId = this->handleId(context);

    auto message = Messages::message(transaction, handleId, body);
    this->_transport->send(message, context);
  }

  void JanusApi::onPluginEvent(const std::shared_ptr<JanusEvent>& event, const std::shared_ptr<Bundle>& context) {
    this->_delegate->onEvent(event, context);
  }

}
