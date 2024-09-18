#include "stimwalker.h"

using namespace STIMWALKER_NAMESPACE;

void onNewData(const devices::CollectorData &newData, int dataId, devices::DataCollection &dataCollection)
{
    utils::Timestamp timestamp;
    std::cout << "New data collected at " << timestamp.timeSinceEpoch() << std::endl;
    dataCollection.addData(dataId, newData);
}

void get_local_time(std::tm *localTime, const std::time_t *now)
{
#ifdef _WIN32
    // Windows (use localtime_s)
    localtime_s(localTime, now);
#else
    // POSIX (use localtime_r)
    localtime_r(now, localTime);
#endif
}

std::string generateFilename()
{
    std::time_t now = std::time(nullptr);
    std::tm localTime;
    get_local_time(&localTime, &now);

    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d-%H-%M-%S");

    return oss.str() + ".csv";
}

int main(int argc, char **argv)
{
    bool isMock = true;
    auto lokomatPtr = devices::makeLokomatDevice(isMock);
    devices::NidaqDevice &lokomat = *lokomatPtr;

    devices::DataCollection dataCollection;
    int dataId = dataCollection.registerNewDataId();

    lokomat.connect();
    int id = lokomat.onNewData([&dataCollection, dataId](const devices::CollectorData &newData)
                               { onNewData(newData, dataId, dataCollection); });

    lokomat.startRecording();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    lokomat.stopRecording();

    lokomat.disconnect();

    nlohmann::json json = dataCollection.serialize();
    dataCollection = dataCollection.deserialize(json);
    int timeIndex = 0;
    std::cout << json[utils::String(id)][timeIndex].dump(2) << std::endl;

    try
    {
        const std::string filename = "emg_data.csv";
        const std::string ip = "127.0.0.1";
        TrignoEMG emg({0, 15}, 2000, "mV", ip);
        emg.start_recording(generateFilename());
        emg.start();

        for (int i = 0; i < 10; ++i)
        {
            auto data = emg.read();
        }

        emg.stop();
        emg.stop_recording();

        std::cout << "Data logging complete. Check " << filename << " for results." << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}