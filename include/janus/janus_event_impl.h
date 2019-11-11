/*!
 * janus-client SDK
 *
 * janus_event_impl.h
 * The Janu Event interface implementation
 * This module implements the janus event interface: an handy object you can use to explore a janus reply message
 *
 * Copyright 2019 Pasquale Boemio <pau@helloiampau.io>
 */

#pragma once

#include <nlohmann/json.hpp>

#include "janus/janus_event.hpp"
#include "janus/janus_data.hpp"
#include "janus/jsep.hpp"
#include "janus/sdp_type.hpp"

namespace Janus {

  class JsepImpl : public Jsep {
    public:
      JsepImpl(const nlohmann::json& jsep);

      std::string sdp();
      SdpType type();

    private:
      SdpType _type;
      std::string _sdp;
  };

  class JanusDataImpl : public JanusData {
    public:
      JanusDataImpl(const nlohmann::json& body);

      std::string getString(const std::string& key, const std::string& fallback);
      int64_t getInt(const std::string& key, int64_t fallback);
      bool getBool(const std::string& key, bool fallback);
      std::shared_ptr<JanusData> getObject(const std::string& key);
      std::vector<std::shared_ptr<JanusData>> getList(const std::string& key);

    private:
      nlohmann::json _content;
  };

  class JanusEventImpl : public JanusEvent {
    public:
      JanusEventImpl(int64_t sender, const nlohmann::json& body);
      JanusEventImpl(int64_t sender, const nlohmann::json& body, const nlohmann::json& sdp);

      int64_t sender();
      std::shared_ptr<Jsep> jsep();
      std::shared_ptr<JanusData> data();

    private:
      int64_t _sender = -1;
      std::shared_ptr<JanusDataImpl> _content;
      std::shared_ptr<Jsep> _jsep = nullptr;
  };

}
