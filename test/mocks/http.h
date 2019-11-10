#pragma once

#include "janus/http.h"

namespace Janus {

  class HttpMock : public Http {
    public:
      MOCK_METHOD1(get, std::shared_ptr<HttpResponse>(const std::string& path));
      MOCK_METHOD2(post, std::shared_ptr<HttpResponse>(const std::string& path, const std::string& body));
  };

}
