#include "d3d11_capture/event.h"

namespace d3d11_capture {

void Event::Clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  is_set_ = false;
}

bool Event::IsSet() {
  std::lock_guard<std::mutex> lock(mutex_);
  return is_set_;
}

void Event::Set() {
  std::lock_guard<std::mutex> lock(mutex_);
  is_set_ = true;
  trigger_.notify_all();
}

void Event::Wait() {
  std::unique_lock<std::mutex> lock(mutex_);
  trigger_.wait(lock, [this]() { return is_set_; });
}

}  // namespace d3d11_capture
