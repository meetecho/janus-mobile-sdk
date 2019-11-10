#include "janus/transport.h"

#include <regex>

namespace Janus {

  /* TransportImpl */

  TransportImpl::TransportImpl(const std::shared_ptr<TransportDelegate>& delegate, const std::shared_ptr<Async>& async) {
    this->_delegate = delegate;
    this->_async = async;

    this->_status = TransportStatus::ON;
  }

  void TransportImpl::sessionId(const std::string& id) {
    std::lock_guard<std::mutex> lock(this->_sessionIdMutex);
    this->_sessionId = id;
  }

  void TransportImpl::close() {
    this->_status = TransportStatus::OFF;
  }

  /* HTTP Transport */

  HttpTransport::HttpTransport(const std::string& url, const std::shared_ptr<TransportDelegate>& delegate, const std::shared_ptr<HttpFactory>& factory, const std::shared_ptr<Async>& async) : TransportImpl(delegate, async) {
    for(int index = 0; index < HTTP_CLIENT_POOL_SIZE; index++) {
      auto client = factory->create(url);
      this->_clients.push(client);
    }
  }

  void HttpTransport::send(const nlohmann::json& message, const std::shared_ptr<Bundle>& context) {
    auto body = message.dump();
    HttpTask task = [=] (const std::string& path, const std::shared_ptr<Http>& client, const std::shared_ptr<HttpTransport>& main) {
      return client->post(path, body);
    };

    this->_sendAsync(task, context);
  }

  void HttpTransport::sessionId(const std::string& id) {
    TransportImpl::sessionId(id);

    auto context = Bundle::create();
    this->_sendAsync(HttpTransport::_loop, context);
  }

  std::shared_ptr<HttpResponse> HttpTransport::_loop(const std::string& path, const std::shared_ptr<Http>& client, const std::shared_ptr<HttpTransport>& main) {
    auto reply = client->get(path);

    auto context = Bundle::create();
    main->_sendAsync(HttpTransport::_loop, context);

    return reply;
  }

  void HttpTransport::_sendAsync(const HttpTask& kernel, const std::shared_ptr<Bundle>& context) {
    auto task = [=] {
      std::unique_lock<std::mutex> notEmptyLock(this->_clientsMutex);
      this->_notEmpty.wait(notEmptyLock, [this] {
        return this->_clients.size() != 0;
      });

      auto client = this->_clients.front();
      this->_clients.pop();

      notEmptyLock.unlock();
      this->_notEmpty.notify_one();

      std::string path = "/";
      {
        std::lock_guard<std::mutex> sessionIdLock(this->_sessionIdMutex);
        if(this->_sessionId.empty() == false) {
          path = path + this->_sessionId;
        }
      }

      if(this->_status == TransportStatus::OFF) {
        return;
      }

      auto reply = kernel(path, client, this->shared_from_this());
      auto content = nlohmann::json::parse(reply->body());
      this->_delegate->onMessage(content, context);

      notEmptyLock.lock();
      this->_clients.push(client);
      notEmptyLock.unlock();
      this->_notEmpty.notify_one();
    };

    this->_async->submit(task);
  }

  /* WS Transport */

  void WebSocketTransport::send(const nlohmann::json& message, const std::shared_ptr<Bundle>& context) {
  }

  void WebSocketTransport::close() {}

  /* Transport Factory */

  std::shared_ptr<Transport> TransportFactoryImpl::create(const std::string& url, const std::shared_ptr<TransportDelegate>& delegate) {
    std::regex HTTP_RXP("^https?:\\/\\/");
    if(std::regex_search(url, HTTP_RXP) == true) {
      auto async = std::make_shared<AsyncImpl>();
      auto factory = std::make_shared<HttpFactoryImpl>();

      return std::make_shared<HttpTransport>(url, delegate, factory, async);
    }

    std::regex WS_RXP("^wss?:\\/\\/");
    if(std::regex_search(url, WS_RXP) == true) {
      return std::make_shared<WebSocketTransport>(url, delegate);
    }

    return nullptr;
  }

}
