/*!
 * janus-client SDK
 *
 * bundle_impl.h
 * The argument bundle implementation
 * This module defines a bundle object you can use to pass arguments to janus commands
 *
 * Copyright 2019 Pasquale Boemio <pau@helloiampau.io>
 */

#pragma once

#include "janus/bundle.hpp"

#include <unordered_map>
#include "janus/constraints.hpp"

#define CONSTRAINTS_KEY "SPiUkrMsbd"

namespace Janus {

  class BundleImpl : public Bundle {
    public:
      void setString(const std::string& key, const std::string& value);
      std::string getString(const std::string& key, const std::string& fallback);

      void setInt(const std::string& key, int64_t value);
      int64_t getInt(const std::string& key, int64_t fallback);

      void setBool(const std::string& key, bool value);
      bool getBool(const std::string& key, bool fallback);

      void setConstraints(const Constraints& constraints);
      Constraints getConstraints();

    private:
      template <typename T>
      void _set(const std::string& key, T value) {
        std::shared_ptr<void> converted = std::make_shared<T>(value);
        this->_values[key] = converted;
      }

      template <typename T>
      T _get(const std::string& key, T fallback) {
        auto value = this->_values[key];

        if(value == nullptr) {
          return fallback;
        }

        return *std::static_pointer_cast<T>(value);
      }

      std::unordered_map<std::string, std::shared_ptr<void>> _values;
  };

}
