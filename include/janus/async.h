/*!
 * janus-client SDK
 *
 * async.h
 * Single thread work queue
 * This class defines a work queue you can use to submit async tasks
 *
 * Copyright 2019 Pasquale Boemio <pau@helloiampau.io>
 */

#pragma once

#include <functional>
#include <queue>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <thread>

#define THREAD_POOL_SIZE 2

namespace Janus {

  typedef std::function<void(void)> Task;

  class Async {
    public:
      virtual void submit(Task task) = 0;
  };

  class AsyncImpl : public Async {
    public:
      AsyncImpl();
      ~AsyncImpl();

      void submit(Task task);
    private:
      bool _isEnabled();

      static void* _loop(AsyncImpl* context);

      std::queue<Task> _queue;
      std::mutex _queueMutex;
      std::condition_variable _notEmpty;

      std::mutex _enabledMutex;
      bool _enabled = true;

      std::thread _threads[THREAD_POOL_SIZE];
  };

}
