#include "dx11/event.h"

namespace dx11 {

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

}  // namespace dx11
