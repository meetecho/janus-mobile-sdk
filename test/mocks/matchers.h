#pragma once

namespace testing {

  MATCHER_P(IsJsonEq, value, "") {
    return value.dump().compare(arg.dump()) == 0;
  }

  MATCHER_P(IsJanusMessage, value, "") {
    return arg.value("janus", "").compare(value) == 0;
  }

  MATCHER_P(HasJsep, value, "") {
    return arg->jsep() != nullptr && arg->jsep()->type() == value;
  }

  MATCHER_P2(BundleHasInt, key, value, "") {
    return arg->getInt(key, -1) == value;
  }

  MATCHER_P2(BundleHasString, key, value, "") {
    return arg->getString(key, "INVALID").compare(value) == 0;
  }

  MATCHER_P2(IsEvent, key, value, "") {
    return arg->data()->getString(key, "").compare(value) == 0;
  }

  MATCHER_P2(IsError, code, reason, "") {
    return arg.code == code && arg.message.compare(reason) == 0;
  }

  MATCHER_P(HasConstraints, value, "") {
    return arg.sdp.send_audio == value.sdp.send_audio &&
      arg.sdp.send_video == value.sdp.send_video &&
      arg.sdp.receive_audio == value.sdp.receive_audio &&
      arg.sdp.receive_video == value.sdp.receive_video &&
      arg.sdp.datachannel == value.sdp.datachannel &&
      arg.video.width == value.video.width &&
      arg.video.height == value.video.height &&
      arg.video.fps == value.video.fps &&
      arg.video.camera == value.video.camera;
  }

}
