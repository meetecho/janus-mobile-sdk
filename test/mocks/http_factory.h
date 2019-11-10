#pragma once

#include "janus/http.h"

namespace Janus {

  class HttpFactoryMock : public HttpFactory {
    public:
      MOCK_METHOD1(create, std::shared_ptr<Http>(const std::string& baseUrl));
  };

}
