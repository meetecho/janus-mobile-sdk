#include "janus/bundle_impl.h"

#include "janus/constraints_builder.hpp"

namespace Janus {

  void BundleImpl::setString(const std::string& key, const std::string& value) {
    this->_set<std::string>(key, value);
  }

  std::string BundleImpl::getString(const std::string& key, const std::string& fallback) {
    return this->_get<std::string>(key, fallback);
  }

  void BundleImpl::setInt(const std::string & key, int64_t value) {
    this->_set<int64_t>(key, value);
  }

  int64_t BundleImpl::getInt(const std::string & key, int64_t fallback) {
    return this->_get<int64_t>(key, fallback);
  }

  void BundleImpl::setBool(const std::string & key, bool value) {
    this->_set<bool>(key, value);
  }

  bool BundleImpl::getBool(const std::string & key, bool fallback) {
    return this->_get<bool>(key, fallback);
  }

  void BundleImpl::setConstraints(const Constraints& constraints) {
    this->_set<Constraints>(CONSTRAINTS_KEY, constraints);
  }

  Constraints BundleImpl::getConstraints() {
    auto builder = ConstraintsBuilder::create();
    auto def = builder->build();

    return this->_get<Constraints>(CONSTRAINTS_KEY, def);;
  }

  std::shared_ptr<Bundle> Bundle::create() {
    return std::make_shared<BundleImpl>();
  }

}
