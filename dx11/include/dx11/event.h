#pragma once

#include <condition_variable>
#include <mutex>

#include "dx11/common.h"

namespace dx11 {

class Event {
 public:
  Event() = default;

  Event(const Event&) = delete;
  Event& operator=(const Event&) = delete;

  Event(Event&&) = delete;
  Event& operator=(Event&&) = delete;

  void Clear();

  bool IsSet();

  void Set();

  void Wait();

 private:
  std::mutex mutex_;
  std::condition_variable trigger_;
  bool is_set_{false};
};

}  // namespace dx11