module;

#include <cstdint>
#include <stdexcept>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

export module capture:timer;

namespace capture {
export class Timer {
 public:
  explicit Timer(std::int32_t target_fps);
  ~Timer();

  Timer(const Timer&) = delete;
  Timer& operator=(const Timer&) = delete;

  Timer(Timer&&) = delete;
  Timer& operator=(Timer&&) = delete;

  void Wait() const;

 private:
  HANDLE timer_{nullptr};

  void SetPeriodic(std::int32_t period) const;
  void Cancel() const;
};
}  // namespace capture

capture::Timer::Timer(std::int32_t target_fps) {
  constexpr auto kFlags{CREATE_WAITABLE_TIMER_HIGH_RESOLUTION};
  constexpr auto kDesiredAccess{TIMER_ALL_ACCESS};
  timer_ = CreateWaitableTimerExW(nullptr, nullptr, kFlags, kDesiredAccess);
  if (timer_ == nullptr)
    throw std::runtime_error{"CreateWaitableTimerExW: Failure"};
  SetPeriodic(1000 / target_fps);
}

capture::Timer::~Timer() {
  Cancel();
  CancelWaitableTimer(timer_);
  CloseHandle(timer_);
}

void capture::Timer::Wait() const {
  if (WaitForSingleObject(timer_, INFINITE) != WAIT_OBJECT_0)
    throw std::runtime_error{"WaitForSingleObject: Failure"};
}

void capture::Timer::SetPeriodic(std::int32_t period) const {
  constexpr std::int64_t kDueTime{0};
  constexpr bool kResume{false};
  if (!SetWaitableTimer(timer_, reinterpret_cast<const LARGE_INTEGER*>(&kDueTime), period, nullptr, nullptr, kResume))
    throw std::runtime_error{"SetWaitableTimer: Failure"};
}

void capture::Timer::Cancel() const {
  if (!CancelWaitableTimer(timer_))
    throw std::runtime_error{"CancelWaitableTimer: Failure"};
}
