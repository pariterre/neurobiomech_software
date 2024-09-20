#include <gtest/gtest.h>
#include <iostream>

#include "Devices/Data/CollectorData.h"
#include "Devices/Data/DataCollection.h"
#include "Devices/NidaqDevice.h"

#include "Utils/String.h"

static double requiredPrecision(1e-10);

using namespace STIMWALKER_NAMESPACE;

// Start the tests

TEST(Data, serialize) {
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
  devices::DataCollection dataCollection = dataCollection.deserialize(json);

  ASSERT_EQ(dataCollection.getNbDataId(), 1);
  const auto &deserializedData = dataCollection.getData(0);
  ASSERT_EQ(deserializedData.size(), 2);

  ASSERT_EQ(deserializedData[0]->getTimestamp(), 2001);
  ASSERT_EQ(deserializedData[0]->getData().size(), 3);
  ASSERT_NEAR(deserializedData[0]->getData()[0], 4.0, requiredPrecision);
  ASSERT_NEAR(deserializedData[0]->getData()[1], 5.0, requiredPrecision);
  ASSERT_NEAR(deserializedData[0]->getData()[2], 6.0, requiredPrecision);

  ASSERT_EQ(deserializedData[1]->getTimestamp(), 2002);
  ASSERT_EQ(deserializedData[1]->getData().size(), 3);
  ASSERT_NEAR(deserializedData[1]->getData()[0], -7.0, requiredPrecision);
  ASSERT_NEAR(deserializedData[1]->getData()[1], 8.0, requiredPrecision);
  ASSERT_NEAR(deserializedData[1]->getData()[2], 9.0, requiredPrecision);
}

TEST(Data, acquire) {
  auto data = devices::DataCollection();
  int dataId = data.registerNewDataId();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  ASSERT_EQ(data.getNbDataId(), 0);

  auto callback = [&data, dataId](const devices::CollectorData &newData) {
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