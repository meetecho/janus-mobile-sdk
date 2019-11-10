#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "janus/constraints_builder_impl.h"

namespace Janus {

  class ConstraintsBuilderImplTest : public testing::Test {
  };

  TEST_F(ConstraintsBuilderImplTest, shouldHaveGoodDefaults) {
    auto constraints = std::make_shared<ConstraintsBuilderImpl>()->build();

    EXPECT_EQ(constraints.sdp.send_audio, true);
    EXPECT_EQ(constraints.sdp.send_video, true);
    EXPECT_EQ(constraints.sdp.receive_audio, true);
    EXPECT_EQ(constraints.sdp.receive_video, true);
    EXPECT_EQ(constraints.sdp.datachannel, true);

    EXPECT_EQ(constraints.video.fps, 30);
    EXPECT_EQ(constraints.video.width, 1280);
    EXPECT_EQ(constraints.video.height, 720);
    EXPECT_EQ(constraints.video.camera, Camera::FRONT);
  }

  TEST_F(ConstraintsBuilderImplTest, shouldToggleTheCamera) {
    auto builder = std::make_shared<ConstraintsBuilderImpl>();

    auto cFront = builder->camera(Camera::FRONT)->build();
    EXPECT_EQ(cFront.video.camera, Camera::FRONT);

    auto cOther = builder->camera(Camera::OTHER)->build();
    EXPECT_EQ(cOther.video.camera, Camera::OTHER);
  }

  TEST_F(ConstraintsBuilderImplTest, shouldToggleDataChannel) {
    auto builder = std::make_shared<ConstraintsBuilderImpl>();

    auto cFalse = builder->datachannel(false)->build();
    EXPECT_EQ(cFalse.sdp.datachannel, false);

    auto cTrue = builder->datachannel(true)->build();
    EXPECT_EQ(cTrue.sdp.datachannel, true);
  }

  TEST_F(ConstraintsBuilderImplTest, shouldToggleSendAudio) {
    auto builder = std::make_shared<ConstraintsBuilderImpl>();

    auto cFalse = builder->send_audio(false)->build();
    EXPECT_EQ(cFalse.sdp.send_audio, false);

    auto cTrue = builder->send_audio(true)->build();
    EXPECT_EQ(cTrue.sdp.send_audio, true);
  }

  TEST_F(ConstraintsBuilderImplTest, shouldToggleSendVideo) {
    auto builder = std::make_shared<ConstraintsBuilderImpl>();

    auto cFalse = builder->send_video(false)->build();
    EXPECT_EQ(cFalse.sdp.send_video, false);

    auto cTrue = builder->send_video(true)->build();
    EXPECT_EQ(cTrue.sdp.send_video, true);
  }

  TEST_F(ConstraintsBuilderImplTest, shouldToggleReceiveAudio) {
    auto builder = std::make_shared<ConstraintsBuilderImpl>();

    auto cFalse = builder->receive_audio(false)->build();
    EXPECT_EQ(cFalse.sdp.receive_audio, false);

    auto cTrue = builder->receive_audio(true)->build();
    EXPECT_EQ(cTrue.sdp.receive_audio, true);
  }

  TEST_F(ConstraintsBuilderImplTest, shouldToggleReceiveVideo) {
    auto builder = std::make_shared<ConstraintsBuilderImpl>();

    auto cFalse = builder->receive_video(false)->build();
    EXPECT_EQ(cFalse.sdp.receive_video, false);

    auto cTrue = builder->receive_video(true)->build();
    EXPECT_EQ(cTrue.sdp.receive_video, true);
  }

  TEST_F(ConstraintsBuilderImplTest, shouldSetAVideoResolution) {
    auto builder = std::make_shared<ConstraintsBuilderImpl>()->video(4, 20,69);
    auto constraints = builder->build();

    EXPECT_EQ(constraints.video.width, 4);
    EXPECT_EQ(constraints.video.height, 20);
    EXPECT_EQ(constraints.video.fps, 69);
  }

  TEST_F(ConstraintsBuilderImplTest, shouldSetSendOnlyMode) {
    auto builder = std::make_shared<ConstraintsBuilderImpl>()->send_only();
    auto c = builder->build();

    EXPECT_EQ(c.sdp.receive_video, false);
    EXPECT_EQ(c.sdp.receive_audio, false);
    EXPECT_EQ(c.sdp.send_video, true);
    EXPECT_EQ(c.sdp.send_audio, true);
  }

  TEST_F(ConstraintsBuilderImplTest, shouldSetReceiveOnlyMode) {
    auto builder = std::make_shared<ConstraintsBuilderImpl>()->receive_only();
    auto c = builder->build();

    EXPECT_EQ(c.sdp.receive_video, true);
    EXPECT_EQ(c.sdp.receive_audio, true);
    EXPECT_EQ(c.sdp.send_video, false);
    EXPECT_EQ(c.sdp.send_audio, false);
  }

  TEST_F(ConstraintsBuilderImplTest, shouldDisableAll) {
    auto builder = std::make_shared<ConstraintsBuilderImpl>()->none();
    auto c = builder->build();

    EXPECT_EQ(c.sdp.receive_video, false);
    EXPECT_EQ(c.sdp.receive_audio, false);
    EXPECT_EQ(c.sdp.send_video, false);
    EXPECT_EQ(c.sdp.send_audio, false);
    EXPECT_EQ(c.sdp.datachannel, false);
  }

  class ConstraintsBuilderTest : public testing::Test {
  };

  TEST_F(ConstraintsBuilderTest, shouldCreateAConstraintBuilderImpl) {
    auto builder = ConstraintsBuilder::create();

    EXPECT_NE(builder, nullptr);
  }

}
