#pragma once

#include "janus/janus_conf.hpp"

namespace Janus {

  class JanusConfMock : public JanusConf {
    public:
      MOCK_METHOD0(url, std::string());
      MOCK_METHOD0(plugin, std::string());
  };

}
