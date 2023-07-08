module;

#include <cstdint>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

export module capture.timer;

export namespace capture {
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
