/*!
 * janus-client SDK
 *
 * http.h
 * An HTTP client
 * This class defines an HTTP client based on libcurl
 *
 * Copyright 2019 Pasquale Boemio <pau@helloiampau.io>
 */

#pragma once

#include <memory>
#include <string>

namespace Janus {

  class HttpResponse {
    public:
      HttpResponse(int status, const std::string& body);

      int status();
      std::string body();
    private:
      int _status;
      std::string _body;
  };

  class Http {
    public:
      virtual std::shared_ptr<HttpResponse> get(const std::string& path) = 0;
      virtual std::shared_ptr<HttpResponse> post(const std::string& path, const std::string& body="") = 0;
  };

  class HttpImpl : public Http {
    public:
      HttpImpl(const std::string& baseUrl);
      ~HttpImpl();

      std::shared_ptr<HttpResponse> get(const std::string& path);
      std::shared_ptr<HttpResponse> post(const std::string& path, const std::string& body="");
    private:
      std::shared_ptr<HttpResponse> _request(const std::string& path, const std::string& method, const std::string& body="");

      static size_t _writeFunction(void* ptr, size_t size, size_t nmemb, std::string* data);

      std::string _baseUrl;
  };

  class HttpFactory {
    public:
      virtual std::shared_ptr<Http> create(const std::string& baseUrl) = 0;
  };

  class HttpFactoryImpl : public HttpFactory {
    public:
      std::shared_ptr<Http> create(const std::string& baseUrl);
  };

}
