#include "stimwalker.h"

int main(int argc, char** argv) {
    // Exit the application
    auto lokomat = stimwalker::devices::NidaqDevice();

    lokomat.connect();
    std::cout << "Connected: " << lokomat.getIsConnected() << std::endl;
     
    lokomat.startRecording();
    std::cout << "Recording: " << lokomat.isRecording() << std::endl;

    lokomat.stopRecording();
    std::cout << "Recording: " << lokomat.isRecording() << std::endl;

    lokomat.disconnect();
    std::cout << "Connected: " << lokomat.getIsConnected() << std::endl;

    lokomat.dispose();

    return 0;
}