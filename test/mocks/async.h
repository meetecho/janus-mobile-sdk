#pragma once

#include "janus/async.h"

namespace Janus {

  class AsyncMock : public Async {
    public:
      MOCK_METHOD1(submit, void(Task task));
  };

}
