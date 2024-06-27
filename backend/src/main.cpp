#include "stimwalker.h"

using namespace STIMWALKER_NAMESPACE;

void onNewData(const devices::CollectorData &newData, int dataId, devices::DataCollection &dataCollection)
{
    utils::Timestamp timestamp;
    std::cout << "New data collected at " << timestamp.timeSinceEpochMillisec() << std::endl;
    dataCollection.addData(dataId, newData);
}

int main(int argc, char **argv)
{
    // Exit the application
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

    return 0;
}