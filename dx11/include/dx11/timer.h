#pragma once

#include <cstdint>

#include "dx11/common.h"

namespace dx11 {

class Timer {
 public:
  Timer(int32_t target_fps);
  ~Timer();

  void Wait();

 private:
  HANDLE timer_{nullptr};

  void SetPeriodic(int32_t period);
  void Cancel();
};

}  // namespace dx11
