module;

#include <cstdint>
#include <stdexcept>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

module capture:timer;

namespace capture {
class Timer {
 public:
  Timer(std::int32_t target_fps);
  ~Timer();

  void Wait();

 private:
  HANDLE timer_{nullptr};

  void SetPeriodic(std::int32_t period);
  void Cancel();
};
}  // namespace capture

// -------- Implementation --------

capture::Timer::Timer(std::int32_t target_fps) {
  constexpr auto kFlags = CREATE_WAITABLE_TIMER_HIGH_RESOLUTION;
  constexpr auto kDesiredAccess = TIMER_ALL_ACCESS;
  timer_ = CreateWaitableTimerExW(nullptr, nullptr, kFlags, kDesiredAccess);
  if (timer_ == nullptr) {
    throw std::runtime_error{"Failed to create timer"};
  }
  SetPeriodic(1000 / target_fps);
}

capture::Timer::~Timer() {
  Cancel();
  CancelWaitableTimer(timer_);
  CloseHandle(timer_);
}

void capture::Timer::Wait() {
  if (WaitForSingleObject(timer_, INFINITE) != WAIT_OBJECT_0) {
    throw std::runtime_error{"Failed to wait for timer"};
  }
}

void capture::Timer::SetPeriodic(std::int32_t period) {
  constexpr auto kDueTime = 0LL;
  constexpr auto kFlags = 0;
  if (!SetWaitableTimer(timer_,
                        reinterpret_cast<const LARGE_INTEGER*>(&kDueTime),
                        period, nullptr, nullptr, kFlags)) {
    throw std::runtime_error{"Failed to set timer"};
  }
}

void capture::Timer::Cancel() {
  if (!CancelWaitableTimer(timer_)) {
    throw std::runtime_error{"Failed to cancel timer"};
  }
}
