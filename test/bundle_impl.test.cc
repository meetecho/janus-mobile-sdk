#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "janus/bundle_impl.h"
#include "janus/constraints_builder_impl.h"

#include "mocks/matchers.h"

using testing::HasConstraints;

namespace Janus {

  class BundleImplTest : public testing::Test {};

  TEST_F(BundleImplTest, shouldStoreAValue) {
    auto bundle = std::make_shared<BundleImpl>();
    bundle->setString("yolo", "my value");

    EXPECT_EQ(bundle->getString("yolo", "DEFAULT"), "my value");
  }

  TEST_F(BundleImplTest, shouldReturnTheDefaultStringOnInvalidKey) {
    auto bundle = std::make_shared<BundleImpl>();
    EXPECT_EQ(bundle->getString("yolo", "DEFAULT"), "DEFAULT");
  }

  TEST_F(BundleImplTest, shouldOverrideStringValues) {
    auto bundle = std::make_shared<BundleImpl>();

    bundle->setString("yolo", "my value");
    bundle->setString("yolo", "my new value");
    EXPECT_EQ(bundle->getString("yolo", "DEFAULT"), "my new value");
  }

  TEST_F(BundleImplTest, shouldStoreAnInt) {
    auto bundle = std::make_shared<BundleImpl>();
    bundle->setInt("yolo", 420);

    EXPECT_EQ(bundle->getInt("yolo", 69), 420);
  }

  TEST_F(BundleImplTest, shouldReturnTheDefaultIntOnInvalidKey) {
    auto bundle = std::make_shared<BundleImpl>();
    EXPECT_EQ(bundle->getInt("yolo", 69), 69);
  }

  TEST_F(BundleImplTest, shouldOverrideIntValues) {
    auto bundle = std::make_shared<BundleImpl>();

    bundle->setInt("yolo", 69);
    bundle->setInt("yolo", 420);
    EXPECT_EQ(bundle->getInt("yolo", -1), 420);
  }

  TEST_F(BundleImplTest, shouldStoreABool) {
    auto bundle = std::make_shared<BundleImpl>();
    bundle->setBool("yolo", true);

    EXPECT_EQ(bundle->getBool("yolo", false), true);
  }

  TEST_F(BundleImplTest, shouldReturnTheDefaultBoolOnInvalidKey) {
    auto bundle = std::make_shared<BundleImpl>();
    EXPECT_EQ(bundle->getInt("yolo", true), true);
  }

  TEST_F(BundleImplTest, shouldOverrideBoolValues) {
    auto bundle = std::make_shared<BundleImpl>();

    bundle->setBool("yolo", true);
    bundle->setBool("yolo", false);
    EXPECT_EQ(bundle->getBool("yolo", true), false);
  }

  TEST_F(BundleImplTest, shouldStoreAnConstraintObject) {
    auto constraints = ConstraintsBuilder::create()->build();

    auto bundle = std::make_shared<BundleImpl>();
    bundle->setConstraints(constraints);

    EXPECT_THAT(bundle->getConstraints(), HasConstraints(constraints));
  }

  TEST_F(BundleImplTest, shouldReturnTheDefaultConstraintObjectIfNotSet) {
    auto defaultConstraints = ConstraintsBuilder::create()->build();

    auto bundle = std::make_shared<BundleImpl>();
    EXPECT_THAT(bundle->getConstraints(), HasConstraints(defaultConstraints));
  }

  class BundleTest : public testing::Test {};

  TEST_F(BundleTest, shouldCreateABundleImplObject) {
    EXPECT_NE(Bundle::create(), nullptr);
  }

}
