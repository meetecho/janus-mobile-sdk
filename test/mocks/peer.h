#pragma once

#include "janus/peer.hpp"
#include "janus/constraints.hpp"

namespace Janus {

  class PeerMock : public Peer {
    public:
      MOCK_METHOD0(close, void());
      MOCK_METHOD2(createOffer, void(const Constraints & constraints, const std::shared_ptr<Bundle>& context));
      MOCK_METHOD2(createAnswer, void(const Constraints & constraints, const std::shared_ptr<Bundle>& context));
      MOCK_METHOD2(setLocalDescription, void(SdpType type, const std::string & sdp));
      MOCK_METHOD2(setRemoteDescription, void(SdpType type, const std::string & sdp));
      MOCK_METHOD3(addIceCandidate, void(const std::string & mid, int32_t index, const std::string & sdp));
  };

}
