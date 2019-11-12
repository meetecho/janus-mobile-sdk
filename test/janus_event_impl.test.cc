#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "janus/janus_event_impl.h"

#include <nlohmann/json.hpp>

namespace Janus {

  class JanusEventImplTest : public testing::Test {
  };

  TEST_F(JanusEventImplTest, shouldExploreTheEventData) {
    nlohmann::json content = {
      { "my string", "a string" },
      { "my int", 420 },
      { "my bool", true },
      { "my obj", { { "parsed", true } } },
      { "my list", { { { "parsed", true } }, { { "other", 69 } } } }
    };

    auto evt = std::make_shared<JanusEventImpl>(69, content);
    auto data = evt->data();

    EXPECT_EQ(data->getString("my string", ""), "a string");
    EXPECT_EQ(data->getInt("my int", 69), 420);
    EXPECT_EQ(data->getBool("my bool", false), true);
    EXPECT_EQ(data->getObject("my obj")->getBool("parsed", false), true);
    EXPECT_EQ(data->getList("my list")[1]->getInt("other", 420), 69);

    EXPECT_EQ(evt->sender(), 69);
  }

  TEST_F(JanusEventImplTest, shouldReturnDefaults) {
    nlohmann::json content = nlohmann::json::object();

    auto evt = std::make_shared<JanusEventImpl>(69, content);
    auto data = evt->data();

    EXPECT_EQ(data->getString("my string", "default"), "default");
    EXPECT_EQ(data->getInt("my int", 69), 69);
    EXPECT_EQ(data->getBool("my bool", false), false);
    EXPECT_EQ(data->getObject("my obj")->getBool("parsed", false), false);
    EXPECT_EQ(data->getList("my list").size(), 0);
  }

  TEST_F(JanusEventImplTest, shouldParseTheJsep) {
    nlohmann::json content = nlohmann::json::object();
    nlohmann::json offerMsg = {
      { "type", "offer" },
      { "sdp", "the sdp" }
    };
    nlohmann::json answerMsg = {
      { "type", "answer" },
      { "sdp", "the sdp" }
    };

    auto offerEvt = std::make_shared<JanusEventImpl>(69, content, offerMsg);
    auto offer = offerEvt->jsep();
    EXPECT_EQ(offer->sdp(), "the sdp");
    EXPECT_EQ(offer->type(), SdpType::OFFER);

    auto answerEvt = std::make_shared<JanusEventImpl>(69, content, answerMsg);
    auto answer = answerEvt->jsep();
    EXPECT_EQ(answer->sdp(), "the sdp");
    EXPECT_EQ(answer->type(), SdpType::ANSWER);
  }

  TEST_F(JanusEventImplTest, shouldReturnNullOnEmptyJsep) {
    nlohmann::json content = nlohmann::json::object();

    auto evt = std::make_shared<JanusEventImpl>(69, content);
    ASSERT_EQ(evt->jsep(), nullptr);
  }

}
