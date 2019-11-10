/*!
 * janus-client SDK
 *
 * random.h
 * Random utilities
 * This module defines a set of utility to generate random strings
 *
 * Copyright 2019 Pasquale Boemio <pau@helloiampau.io>
 */

#pragma once

#include <string>

namespace Janus {

  class Random {
    public:
      virtual std::string generate() = 0;
  };

  class RandomImpl : public Random {
    public:
      std::string generate();
  };

}
