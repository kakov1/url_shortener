#pragma once

#include "entities/entities.hpp"
#include "logic/url_service.hpp"

namespace shortener {

class ISession {
public:
  virtual void handle_session() = 0;

  virtual ~ISession() = default;
};
} // namespace shortener