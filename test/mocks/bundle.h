#pragma once

#include "janus/bundle.hpp"

#include "janus/constraints.hpp"

namespace Janus {

  class BundleMock : public Bundle {
    public:
      MOCK_METHOD2(setString, void(const std::string& key, const std::string& value));
      MOCK_METHOD2(getString, std::string(const std::string& key, const std::string& fallback));

      MOCK_METHOD2(setInt, void(const std::string& key, int64_t value));
      MOCK_METHOD2(getInt, int64_t(const std::string& key, int64_t fallback));

      MOCK_METHOD2(setBool, void(const std::string& key, bool value));
      MOCK_METHOD2(getBool, bool(const std::string& key, bool fallback));

      MOCK_METHOD1(setConstraints, void(const Constraints& constraints));
      MOCK_METHOD0(getConstraints, Constraints());
  };

}
