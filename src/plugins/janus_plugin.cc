#include "janus/plugins/janus_plugin.h"

namespace Janus {

  JanusPlugin::JanusPlugin(int64_t handleId, const std::shared_ptr<PluginCommandDelegate>& delegate, const std::shared_ptr<PeerFactory>& peerFactory, const std::shared_ptr<Protocol>& owner) {
    this->_delegate = delegate;
    this->_peerFactory = peerFactory;
    this->_owner = owner;
    this->_handleId = handleId;
  }

  void JanusPlugin::onClose() {
    if(this->_peer == nullptr) {
      return;
    }

    this->onHangup("");
  }

  void JanusPlugin::onHangup(const std::string& reason) {
    this->_peer->close();
    this->_peer = nullptr;
  }

}
