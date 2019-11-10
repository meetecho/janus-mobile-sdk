#include "janus/constraints_builder_impl.h"

#include "janus/video_constraints.hpp"
#include "janus/sdp_constraints.hpp"

namespace Janus {

  /* ConstraintsBuilderimpl */

  Constraints ConstraintsBuilderImpl::build() {
    auto sdp = SdpConstraints(this->_send_audio, this->_send_video, this->_receive_audio, this->_receive_video, this->_datachannel);

    auto video = VideoConstraints(this->_width, this->_height, this->_fps, this->_camera);

    return Constraints(sdp, video);
  }

  std::shared_ptr<ConstraintsBuilder> ConstraintsBuilderImpl::datachannel(bool enable) {
    this->_datachannel = enable;

    return this->shared_from_this();
  }

  std::shared_ptr<ConstraintsBuilder> ConstraintsBuilderImpl::send_audio(bool enable) {
    this->_send_audio = enable;

    return this->shared_from_this();
  }

  std::shared_ptr<ConstraintsBuilder> ConstraintsBuilderImpl::send_video(bool enable) {
    this->_send_video = enable;

    return this->shared_from_this();
  }

  std::shared_ptr<ConstraintsBuilder> ConstraintsBuilderImpl::receive_audio(bool enable) {
    this->_receive_audio = enable;

    return this->shared_from_this();
  }

  std::shared_ptr<ConstraintsBuilder> ConstraintsBuilderImpl::receive_video(bool enable) {
    this->_receive_video = enable;

    return this->shared_from_this();
  }

  std::shared_ptr<ConstraintsBuilder> ConstraintsBuilderImpl::video(int32_t width, int32_t height, int32_t fps) {
    this->_width = width;
    this->_height = height;
    this->_fps = fps;

    return this->shared_from_this();
  }

  std::shared_ptr<ConstraintsBuilder> ConstraintsBuilderImpl::camera(Camera camera) {
    this->_camera = camera;

    return this->shared_from_this();
  }

  std::shared_ptr<ConstraintsBuilder> ConstraintsBuilderImpl::send_only() {
    this->send_audio(true);
    this->send_video(true);
    this->receive_audio(false);
    this->receive_video(false);

    return this->shared_from_this();
  }

  std::shared_ptr<ConstraintsBuilder> ConstraintsBuilderImpl::receive_only() {
    this->send_audio(false);
    this->send_video(false);
    this->receive_audio(true);
    this->receive_video(true);

    return this->shared_from_this();
  }

  std::shared_ptr<ConstraintsBuilder> ConstraintsBuilderImpl::none() {
    this->send_audio(false);
    this->send_video(false);
    this->receive_audio(false);
    this->receive_video(false);
    this->datachannel(false);

    return this->shared_from_this();
  }

  /* ConstraintsBuilder */

  std::shared_ptr<ConstraintsBuilder> ConstraintsBuilder::create() {
    auto builder = std::make_shared<ConstraintsBuilderImpl>();

    return builder;
  }

}
