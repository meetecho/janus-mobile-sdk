#include "janus/janus_event_impl.h"

namespace Janus {

  /* Januseventimpl */

  JanusEventImpl::JanusEventImpl(int64_t sender, const nlohmann::json& body) {
    auto content = std::make_shared<JanusDataImpl>(body);
    this->_content = content;
    this->_sender = sender;
  }

  JanusEventImpl::JanusEventImpl(int64_t sender, const nlohmann::json& body, const nlohmann::json& sdp) : JanusEventImpl(sender, body) {
    auto jsep = std::make_shared<JsepImpl>(sdp);
    this->_jsep = jsep;
  }

  std::shared_ptr<JanusData> JanusEventImpl::data() {
    return this->_content;
  }

  std::shared_ptr<Jsep> JanusEventImpl::jsep() {
    return this->_jsep;
  }

  int64_t JanusEventImpl::sender() {
    return this->_sender;
  }

  /* JanusDataImpl */

  JanusDataImpl::JanusDataImpl(const nlohmann::json& body) {
    this->_content = body;
  }

  std::string JanusDataImpl::getString(const std::string& key, const std::string& fallback) {
    return this->_content.value(key, fallback);
  }

  int64_t JanusDataImpl::getInt(const std::string& key, int64_t fallback) {
    return this->_content.value(key, fallback);
  }

  bool JanusDataImpl::getBool(const std::string& key, bool fallback) {
    return this->_content.value(key, fallback);
  }

  std::shared_ptr<JanusData> JanusDataImpl::getObject(const std::string& key) {
    auto child = this->_content.value(key, nlohmann::json::object());

    return std::make_shared<JanusDataImpl>(child);
  }

  std::vector<std::shared_ptr<JanusData>> JanusDataImpl::getList(const std::string & key) {
    std::vector<std::shared_ptr<JanusData>> parsed({});

    std::vector<nlohmann::json> items = this->_content.value(key, std::vector<nlohmann::json>({}));
    for(unsigned index = 0; index < items.size(); index++) {
      auto child = std::make_shared<JanusDataImpl>(items[index]);
      parsed.push_back(child);
    }

    return parsed;
  }

  /* Jsepimpl */

  JsepImpl::JsepImpl(const nlohmann::json& jsep) {
    this->_sdp = jsep.value("sdp", "");
    this->_type = jsep.value("type", "") == "offer" ? SdpType::OFFER : SdpType::ANSWER;
  }

  std::string JsepImpl::sdp() {
    return this->_sdp;
  }

  SdpType JsepImpl::type() {
    return this->_type;
  }

}
