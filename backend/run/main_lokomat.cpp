#include "stimwalker.h"

using namespace STIMWALKER_NAMESPACE;

void onNewData(data::TimeSeries &timeSeries, const data::DataPoint &newData) {
  timeSeries.add(newData);
}

void get_local_time(std::tm *localTime, const std::time_t *now) {
#ifdef _WIN32
  // Windows (use localtime_s)
  localtime_s(localTime, now);
#else
  // POSIX (use localtime_r)
  localtime_r(now, localTime);
#endif
}

std::string generateFilename() {
  std::time_t now = std::time(nullptr);
  std::tm localTime;
  get_local_time(&localTime, &now);

  std::ostringstream oss;
  oss << std::put_time(&localTime, "%Y-%m-%d-%H-%M-%S");

  return oss.str() + ".csv";
}

int main(int argc, char **argv) {
  auto lokomat = devices::LokomatDevice();

  lokomat.connect();
  lokomat.startRecording();
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  lokomat.stopRecording();
  lokomat.disconnect();

  return 0;
}