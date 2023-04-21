#include "ets2ld/application.h"
#include "ets2ld/settings.h"

int main(int argc, char** argv) {
  ets2ld::Settings settings = ets2ld::Settings::ParseArgs(argc, argv);
  ets2ld::Application ets2_lane_detection{settings};
  ets2_lane_detection.Run();
}
