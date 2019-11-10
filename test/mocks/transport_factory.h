#pragma once

#include "janus/transport.h"

namespace Janus {

  class TransportFactoryMock : public TransportFactory {
    public:
      MOCK_METHOD2(create, std::shared_ptr<Transport>(const std::string& url, const std::shared_ptr<TransportDelegate>& delegate));
  };

}
