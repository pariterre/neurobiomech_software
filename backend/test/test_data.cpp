#include <iostream>
#include <gtest/gtest.h>

#include "Devices/Data/DataCollection.h"
#include "Devices/Data/CollectorData.h"
#include "Devices/NidaqDevice.h"

#include "Utils/String.h"

static double requiredPrecision(1e-10);

using namespace STIMWALKER_NAMESPACE;

// Start the tests

TEST(Data, serialize)
{
    auto data = devices::DataCollection();
    int dataId = data.registerNewDataId();

    devices::CollectorData newData(1001, {1.0, 2.0, 3.0});
    data.addData(dataId, newData);

    auto json = data.serialize();
    auto dataIdString = utils::String(dataId);
    ASSERT_EQ(json.size(), 1);
    ASSERT_EQ(json[dataIdString].size(), 1);
    ASSERT_EQ(json[dataIdString][0]["timestamp"], 1001);
    ASSERT_EQ(json[dataIdString][0]["data"].size(), 3);
    ASSERT_NEAR(json[dataIdString][0]["data"][0], 1.0, requiredPrecision);
    ASSERT_NEAR(json[dataIdString][0]["data"][1], 2.0, requiredPrecision);
    ASSERT_NEAR(json[dataIdString][0]["data"][2], 3.0, requiredPrecision);

    auto jsonAsString = json.dump(2);
    ASSERT_STREQ(
        jsonAsString.c_str(),
        "{\n"
        "  \"0\": [\n"
        "    {\n"
        "      \"data\": [\n"
        "        1.0,\n"
        "        2.0,\n"
        "        3.0\n"
        "      ],\n"
        "      \"timestamp\": 1001\n"
        "    }\n"
        "  ]\n"
        "}");
}

TEST(Data, deserialize)
{
    // TODO
}

TEST(Data, acquire)
{
    auto data = devices::DataCollection();
    int dataId = data.registerNewDataId();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    ASSERT_EQ(data.getNbDataId(), 0);

    auto callback = [&data, dataId](const devices::CollectorData &newData)
    {
        data.addData(dataId, newData);
    };

    auto nidaq = devices::NidaqDeviceMock(4, 1000);
    nidaq.onNewData(callback);
    nidaq.connect();
    nidaq.startRecording();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    nidaq.dispose();

    ASSERT_EQ(data.getNbDataId(), 1);
    const auto &dataFirstDeviceAfter = data.getData(dataId);
    ASSERT_GE(dataFirstDeviceAfter.size(), 1);
}