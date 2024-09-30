#include <gtest/gtest.h>
#include <iostream>

#include "Devices/Data/DataDevices.h"
#include "Devices/Data/TimeSeries.h"
#include "Devices/NidaqDevice.h"

#include "Utils/String.h"

static double requiredPrecision(1e-10);

using namespace STIMWALKER_NAMESPACE;

// Start the tests

TEST(Data, serialize) {
  auto devices = devices::data::DataDevices();
  devices.newDevice("0");

  devices::data::DataPoint newData(1001, {1.0, 2.0, 3.0});
  devices["0"].add(newData);

  auto json = devices.serialize();
  ASSERT_EQ(json.size(), 1);
  ASSERT_EQ(json["0"].size(), 1);
  ASSERT_EQ(json["0"][0]["timestamp"], 1001);
  ASSERT_EQ(json["0"][0]["data"].size(), 3);
  ASSERT_NEAR(json["0"][0]["data"][0], 1.0, requiredPrecision);
  ASSERT_NEAR(json["0"][0]["data"][1], 2.0, requiredPrecision);
  ASSERT_NEAR(json["0"][0]["data"][2], 3.0, requiredPrecision);

  auto jsonAsString = json.dump(2);
  ASSERT_STREQ(jsonAsString.c_str(), "{\n"
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

TEST(Data, deserialize) {
  nlohmann::json json = R"({
        "0": [
            {
                "data": [4.0, 5.0, 6.0],
                "timestamp": 2001
            },
            {
                "data": [-7.0, 8.0, 9.0],
                "timestamp": 2002
            }
        ]
    })"_json;
  devices::data::DataDevices devices =
      devices::data::DataDevices::deserialize(json);

  ASSERT_EQ(devices.size(), 1);
  auto &timeSeries = devices["0"];
  ASSERT_EQ(timeSeries.size(), 2);

  ASSERT_EQ(timeSeries[0].getTimestamp(), 2001);
  ASSERT_EQ(timeSeries[0].getData().size(), 3);
  ASSERT_NEAR(timeSeries[0].getData()[0], 4.0, requiredPrecision);
  ASSERT_NEAR(timeSeries[0].getData()[1], 5.0, requiredPrecision);
  ASSERT_NEAR(timeSeries[0].getData()[2], 6.0, requiredPrecision);

  ASSERT_EQ(timeSeries[1].getTimestamp(), 2002);
  ASSERT_EQ(timeSeries[1].getData().size(), 3);
  ASSERT_NEAR(timeSeries[1].getData()[0], -7.0, requiredPrecision);
  ASSERT_NEAR(timeSeries[1].getData()[1], 8.0, requiredPrecision);
  ASSERT_NEAR(timeSeries[1].getData()[2], 9.0, requiredPrecision);
}

TEST(Data, acquire) {
  auto devices = devices::data::DataDevices();
  devices.newDevice("Dummy");
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  auto callback = [&devices](const devices::data::DataPoint &newData) {
    devices["Dummy"].add(newData);
  };

  // TODO Redo the Mock
  auto nidaq = devices::NidaqDevice(4, 1000);
  nidaq.onNewData.listen(callback);
  nidaq.connect();
  nidaq.startRecording();
  std::this_thread::sleep_for(std::chrono::milliseconds(1));

  ASSERT_EQ(devices["Dummy"].size(), 1);
  const auto &dataFirstDeviceAfter = devices["Dummy"];
  ASSERT_GE(dataFirstDeviceAfter.size(), 1);
}