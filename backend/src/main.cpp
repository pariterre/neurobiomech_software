#include "stimwalker.h"

using namespace STIMWALKER_NAMESPACE;

void printDevice(const devices::Device& device){
    std::cout << "Is connected: " << device.getIsConnected() << std::endl;
}

void printCollector(const devices::Collector& collector){
    std::cout << "Nb channels: " << collector.getNbChannels() << std::endl;
    std::cout << "Frame rate: " << collector.getFrameRate() << std::endl;
    std::cout << "Is recording: " << collector.isRecording() << std::endl;
}

int main(int argc, char** argv) {
    // Exit the application
    bool isMock = true;
    auto lokomat = devices::makeLokomat(isMock);
    printDevice(lokomat);
    printCollector(lokomat);
    return 0;
}