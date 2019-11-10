#include "janus/async.h"

namespace Janus {

  AsyncImpl::AsyncImpl() {
    for(unsigned index = 0; index < THREAD_POOL_SIZE; index++) {
      this->_threads[index] = std::thread(this->_loop, this);
    }
  }

  AsyncImpl::~AsyncImpl() {
    {
      std::lock_guard<std::mutex> lock(this->_enabledMutex);
      this->_enabled = false;
    }

    this->_notEmpty.notify_all();

    for(unsigned index = 0; index < THREAD_POOL_SIZE; index++) {
      this->_threads[index].join();
    }
  }

  void AsyncImpl::submit(Task task) {
    std::lock_guard<std::mutex> lock(this->_queueMutex);
    this->_queue.push(task);

    this->_notEmpty.notify_one();
  }

  bool AsyncImpl::_isEnabled() {
    std::lock_guard<std::mutex> lock(this->_enabledMutex);
    return this->_enabled;
  }

  void* AsyncImpl::_loop(AsyncImpl* context) {
    while(context->_isEnabled() == true) {
      std::unique_lock<std::mutex> lock(context->_queueMutex);
      context->_notEmpty.wait(lock, [context] {
        return context->_queue.size() != 0 || context->_isEnabled() == false;
      });

      if(context->_isEnabled() == false) {
        return nullptr;
      }

      Task task = context->_queue.front();
      context->_queue.pop();

      lock.unlock();
      context->_notEmpty.notify_one();

      task();
    }

    return nullptr;
  }

}
