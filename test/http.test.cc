#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <curl/curl.h>

#include "janus/http.h"

using testing::HasSubstr;

namespace Janus {

  class HttpTest : public testing::Test {
  };

  TEST_F(HttpTest, shouldForwardHttpCodes) {
    auto http = std::make_shared<HttpImpl>("http://httpbin");
    auto response = http->get("/post");

    EXPECT_EQ(response->status(), 405);
  }

  TEST_F(HttpTest, shouldForwardACurlError) {
    auto http = std::make_shared<HttpImpl>("yolo://fake");
    auto response = http->get("/post");

    EXPECT_EQ(response->status(), CURLE_UNSUPPORTED_PROTOCOL);
  }

  TEST_F(HttpTest, shouldPerformAGetRequest) {
    auto http = std::make_shared<HttpImpl>("http://httpbin");
    auto response = http->get("/get");

    EXPECT_EQ(response->status(), 200);
    EXPECT_THAT(response->body(), HasSubstr("http://httpbin/get"));
    EXPECT_THAT(response->body(), HasSubstr("Janus Native HTTP Client"));
  }

  TEST_F(HttpTest, shouldPerformAPostRequest) {
    auto http = std::make_shared<HttpImpl>("http://httpbin");
    auto response = http->post("/post");

    EXPECT_EQ(response->status(), 200);
    EXPECT_THAT(response->body(), HasSubstr("http://httpbin/post"));
    EXPECT_THAT(response->body(), HasSubstr("Janus Native HTTP Client"));
  }

  TEST_F(HttpTest, shouldSendJsonDataViaPost) {
    auto http = std::make_shared<HttpImpl>("http://httpbin");
    auto response = http->post("/post", "{ \"data\": \"my yolo data\" }");

    EXPECT_THAT(response->body(), HasSubstr("my yolo data"));
  }

  class HttpFactoryTest : public testing::Test {
  };

  TEST_F(HttpFactoryTest, shouldCreateAnHttpClient) {
    auto factory = std::make_shared<HttpFactoryImpl>();
    auto http = factory->create("the url");

    EXPECT_NE(http, nullptr);
  }
}
