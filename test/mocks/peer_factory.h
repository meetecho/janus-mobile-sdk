#pragma once

#include "janus/peer_factory.hpp"

namespace Janus {

  class PeerFactoryMock : public PeerFactory {
    public:
      MOCK_METHOD2(create, std::shared_ptr<Peer>(int64_t id, const std::shared_ptr<Protocol>& owner));
  };

}
