#include "janus/janus_impl.h"

#include "janus/protocol.hpp"

namespace Janus {

  /* JanusImpl */

  JanusImpl::JanusImpl(const std::shared_ptr<JanusConf>& conf, const std::shared_ptr<Platform>& platform, const std::shared_ptr<ProtocolDelegate>& delegate) {
    this->_conf = conf;
    this->_platform = std::static_pointer_cast<PlatformImpl>(platform);
    this->_delegate = delegate;
  }

  void JanusImpl::init() {
    if(this->_protocol != nullptr) {
      return;
    }

    this->_protocol = this->_platform->protocol();
    this->_protocol->init(this->_conf, this->_platform, this->_delegate);
  }

  void JanusImpl::close() {
    if(this->_protocol == nullptr) {
      return;
    }

    this->_protocol->close();
    this->_protocol = nullptr;
  }

  void JanusImpl::hangup() {
    if(this->_protocol == nullptr) {
      return;
    }

    this->_protocol->hangup();
  }

  void JanusImpl::dispatch(const std::string& command, const std::shared_ptr<Bundle>& payload) {
    auto protocol = this->_platform->protocol();
    protocol->dispatch(command, payload);
  }

  /* Janus */

  std::shared_ptr<Janus> Janus::create(const std::shared_ptr<JanusConf>& conf, const std::shared_ptr<Platform>& platform, const std::shared_ptr<ProtocolDelegate>& delegate) {
    return std::make_shared<JanusImpl>(conf, platform, delegate);
  }

}
