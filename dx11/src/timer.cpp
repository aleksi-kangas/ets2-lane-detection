#include "dx11/timer.h"

#include <stdexcept>

namespace dx11 {

Timer::Timer(int32_t target_fps) {
  constexpr auto kFlags = CREATE_WAITABLE_TIMER_HIGH_RESOLUTION;
  constexpr auto kDesiredAccess = TIMER_ALL_ACCESS;
  timer_ = CreateWaitableTimerExW(nullptr, nullptr, kFlags, kDesiredAccess);
  if (timer_ == nullptr) {
    throw std::runtime_error{"Failed to create timer"};
  }
  SetPeriodic(1000 / target_fps);
}

Timer::~Timer() {
  Cancel();
  CancelWaitableTimer(timer_);
  CloseHandle(timer_);
}

void Timer::Wait() {
  if (WaitForSingleObject(timer_, INFINITE) != WAIT_OBJECT_0) {
    throw std::runtime_error{"Failed to wait for timer"};
  }
}

void Timer::SetPeriodic(int32_t period) {
  constexpr auto kDueTime = 0LL;
  constexpr auto kFlags = 0;
  if (!SetWaitableTimer(timer_,
                        reinterpret_cast<const LARGE_INTEGER*>(&kDueTime),
                        period, nullptr, nullptr, kFlags)) {
    throw std::runtime_error{"Failed to set timer"};
  }
}

void Timer::Cancel() {
  if (!CancelWaitableTimer(timer_)) {
    throw std::runtime_error{"Failed to cancel timer"};
  }
}

}  // namespace dx11
