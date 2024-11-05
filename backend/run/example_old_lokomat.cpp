#include "stimwalker.h"

using namespace STIMWALKER_NAMESPACE;

int main(int argc, char **argv) {
  auto lokomat = devices::LokomatDevice();

  lokomat.connect();
  lokomat.startDataStreaming();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  lokomat.stopDataStreaming();
  lokomat.disconnect();

  return 0;
}