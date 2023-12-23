#include <compare>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

import ets2ld;

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
  ets2ld::Application app{};
  app.Run();
}
