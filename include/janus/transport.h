/*!
 * janus-client SDK
 *
 * transport.h
 * Janus tranports implementation
 * This module defines twot different kinds of janus transport: HTTP and WebSocket
 *
 * Copyright 2019 Pasquale Boemio <pau@helloiampau.io>
 */

#pragma once

#define HTTP_CLIENT_POOL_SIZE 2

#include <memory>
#include <queue>
#include <nlohmann/json.hpp>

#include "janus/http.h"
#include "janus/async.h"
#include "janus/bundle.hpp"

namespace Janus {

  class HttpTransport;

  using HttpTask = std::function<std::shared_ptr<HttpResponse>(const std::string&, const std::shared_ptr<Http>&, const std::shared_ptr<HttpTransport>&)>;

  class TransportDelegate {
    public:
      virtual void onMessage(const nlohmann::json& message, const std::shared_ptr<Bundle>& context) = 0;
  };

  enum TransportType { HTTP, WS };
  enum TransportStatus { ON, OFF };

  class Transport {
    public:
      virtual void sessionId(const std::string& id) = 0;
      virtual void close() = 0;

      virtual TransportType type() = 0;
      virtual void send(const nlohmann::json& message, const std::shared_ptr<Bundle>& context) = 0;
  };

  class TransportImpl : public Transport {
    public:
      TransportImpl(const std::shared_ptr<TransportDelegate>& delegate, const std::shared_ptr<Async>& async);
      void sessionId(const std::string& id);
      void close();

    protected:
      TransportStatus _status = TransportStatus::OFF;

      std::shared_ptr<TransportDelegate> _delegate;

      std::string _sessionId = "";
      std::mutex _sessionIdMutex;

      std::shared_ptr<Async> _async;
  };

  class HttpTransport : public TransportImpl, public std::enable_shared_from_this<HttpTransport> {
    public:
      HttpTransport(const std::string& url, const std::shared_ptr<TransportDelegate>& delegate, const std::shared_ptr<HttpFactory>& factory, const std::shared_ptr<Async>& async);

      TransportType type() {
        return TransportType::HTTP;
      }

      void send(const nlohmann::json& message, const std::shared_ptr<Bundle>& context);
      void sessionId(const std::string& id);
    private:
      void _sendAsync(const HttpTask& kernel, const std::shared_ptr<Bundle>& context);

      static std::shared_ptr<HttpResponse> _loop(const std::string& path, const std::shared_ptr<Http>& client, const std::shared_ptr<HttpTransport>& main);

      std::queue<std::shared_ptr<Http>> _clients;
      std::mutex _clientsMutex;
      std::condition_variable _notEmpty;
  };

  class WebSocketTransport : public TransportImpl {
    public:
      WebSocketTransport(const std::string& url, const std::shared_ptr<TransportDelegate>& delegate) : TransportImpl(delegate, nullptr) {}
      TransportType type() {
        return TransportType::WS;
      }

      void send(const nlohmann::json& message, const std::shared_ptr<Bundle>& context);
      void close();
  };

  class TransportFactory {
    public:
      virtual std::shared_ptr<Transport> create(const std::string& url, const std::shared_ptr<TransportDelegate>& delegate) = 0;
  };

  class TransportFactoryImpl : public TransportFactory {
    public:
      std::shared_ptr<Transport> create(const std::string& url, const std::shared_ptr<TransportDelegate>& delegate);
  };

}
