#pragma once

#include "janus/random.h"

namespace Janus {

  class RandomMock : public Random {
    public:
      MOCK_METHOD0(generate, std::string());
  };

}
