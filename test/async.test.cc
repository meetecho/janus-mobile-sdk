#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "janus/async.h"

using testing::ElementsAre;

namespace Janus {

  class AsyncTest : public testing::Test {
  };

  TEST_F(AsyncTest, shouldExecuteTasksInBackground) {
    std::mutex mutex;

    std::vector<int> results = { -1, -1 };
    auto async = std::make_shared<AsyncImpl>();

    async->submit([&] {
      std::lock_guard<std::mutex> lock(mutex);
      results[0] = 200;
    });

    async->submit([&] {
      std::lock_guard<std::mutex> lock(mutex);
      results[1] = 201;
    });

    usleep(10000);

    std::lock_guard<std::mutex> lock(mutex);
    EXPECT_THAT(results, ElementsAre(200, 201));
  }

}
