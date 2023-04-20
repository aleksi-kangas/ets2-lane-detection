#include "ets2ld/application.h"
#include "ets2ld/arguments.h"

int main(int argc, char** argv) {
  ets2ld::Arguments arguments = ets2ld::Arguments::Parse(argc, argv);
  ets2ld::Application ets2_lane_detection{arguments};
  ets2_lane_detection.Run();
}
