/*!
 * janus-client SDK
 *
 * constraints_builder_impl.h
 * RTC constraints builder
 * This class defines a fluent API you can use to handly create a RTC constraints object
 *
 * Copyright 2019 Pasquale Boemio <pau@helloiampau.io>
 */

#pragma once

#include "janus/constraints_builder.hpp"

#include "janus/constraints.hpp"

namespace Janus {

  class ConstraintsBuilderImpl : public ConstraintsBuilder, public std::enable_shared_from_this<ConstraintsBuilderImpl> {
    public:
      std::shared_ptr<ConstraintsBuilder> datachannel(bool enable);
      std::shared_ptr<ConstraintsBuilder> send_audio(bool enable);
      std::shared_ptr<ConstraintsBuilder> send_video(bool enable);
      std::shared_ptr<ConstraintsBuilder> receive_audio(bool enable);
      std::shared_ptr<ConstraintsBuilder> receive_video(bool enable);
      std::shared_ptr<ConstraintsBuilder> video(int32_t width, int32_t height, int32_t fps);
      std::shared_ptr<ConstraintsBuilder> camera(Camera camera);
      std::shared_ptr<ConstraintsBuilder> send_only();
      std::shared_ptr<ConstraintsBuilder> receive_only();
      std::shared_ptr<ConstraintsBuilder> none();

      Constraints build();

    private:
      bool _datachannel = true;
      bool _send_audio = true;
      bool _send_video = true;
      bool _receive_audio = true;
      bool _receive_video = true;
      int32_t _width = 1280;
      int32_t _height = 720;
      int32_t _fps = 30;
      Camera _camera = Camera::FRONT;
  };

}
