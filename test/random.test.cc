#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "janus/random.h"

using testing::MatchesRegex;

namespace Janus {

  class RandomImplTest : public testing::Test {};

  TEST_F(RandomImplTest, shouldGenerateA16CharsAlphanumericRandomString) {
    auto random = std::make_shared<RandomImpl>();

    std::string first = random->generate();
    std::string second = random->generate();

    EXPECT_NE(first, second);
    EXPECT_THAT(first, MatchesRegex("^[a-zA-Z0-9]{16}$"));
    EXPECT_THAT(second, MatchesRegex("^[a-zA-Z0-9]{16}$"));
  }

}
