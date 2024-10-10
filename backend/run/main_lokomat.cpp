#include "stimwalker.h"

using namespace STIMWALKER_NAMESPACE;

int main(int argc, char **argv) {
  auto lokomat = devices::LokomatDevice();

  lokomat.connect();
  lokomat.startRecording();
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  lokomat.stopRecording();
  lokomat.disconnect();

  return 0;
}