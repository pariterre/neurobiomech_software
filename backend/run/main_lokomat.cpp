#include "stimwalker.h"

using namespace STIMWALKER_NAMESPACE;

void onNewData(devices::data::TimeSeries &timeSeries,
               const devices::data::DataPoint &newData) {
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
  bool isMock = true;
  auto lokomatPtr = devices::makeLokomatDevice(isMock);
  devices::NidaqDevice &lokomat = *lokomatPtr;

  devices::data::DataDevices devices;
  devices.newDevice("lokomat");

  lokomat.connect();
  lokomat.onNewData.listen([&devices](const devices::data::DataPoint &newData) {
    onNewData(devices["lokomat"], newData);
  });

  lokomat.startRecording();
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  lokomat.stopRecording();

  lokomat.disconnect();

  std::string filename = "emg_data.csv";
  std::string host = "127.0.0.1";
  devices::DelsysEmgDevice emg(std::make_pair<size_t>(0, 15), 2000, host);
  emg.connect();
  emg.startRecording();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  emg.stopRecording();
  emg.disconnect();

  return 0;
}