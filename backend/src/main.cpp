#include "stimwalker.h"

using namespace STIMWALKER_NAMESPACE;

void printDevice(const devices::Device &device)
{
    std::cout << "Is connected: " << device.getIsConnected() << std::endl;
}

void printCollector(const devices::Collector &collector)
{
    std::cout << "Nb channels: " << collector.getNbChannels() << std::endl;
    std::cout << "Frame rate: " << collector.getFrameRate() << std::endl;
    std::cout << "Is recording: " << collector.isRecording() << std::endl;
}

void onDataCollected(const devices::CollectorData &newData)
{
    std::cout << "New data collected, yeah!" << std::endl;
}

int main(int argc, char **argv)
{
    // Exit the application
    bool isMock = true;
    auto lokomatPtr = devices::makeLokomatDevice(isMock);
    devices::NidaqDevice &lokomat = *lokomatPtr;

    printDevice(lokomat);
    printCollector(lokomat);

    lokomat.connect();
    int id = lokomat.onNewData(onDataCollected);
    lokomat.startRecording();

    std::this_thread::sleep_for(std::chrono::seconds(1)); // Let it run for 5 seconds
    lokomat.removeListener(id);
    std::this_thread::sleep_for(std::chrono::seconds(1)); // Let it run for 5 seconds

    lokomat.stopRecording();
    lokomat.disconnect();

    return 0;
}