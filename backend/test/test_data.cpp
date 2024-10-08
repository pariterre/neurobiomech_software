#include <gtest/gtest.h>
#include <iostream>

#include "Data/TimeSeries.h"
#include "Devices/DataDevices.h"

static double requiredPrecision(1e-10);

using namespace STIMWALKER_NAMESPACE;

TEST(DataPoint, Constructors) {
  auto data = devices::data::DataPoint({1.0, 2.0, 3.0});
  ASSERT_EQ(data.getTimestamp(), -1);
  ASSERT_EQ(data.size(), 3);
  ASSERT_NEAR(data[0], 1.0, requiredPrecision);
  ASSERT_NEAR(data[1], 2.0, requiredPrecision);
  ASSERT_NEAR(data[2], 3.0, requiredPrecision);

  auto data2 = devices::data::DataPoint(1000, {4.0, 5.0, 6.0});
  ASSERT_EQ(data2.getTimestamp(), 1000);
  ASSERT_EQ(data2.size(), 3);
  ASSERT_NEAR(data2[0], 4.0, requiredPrecision);
  ASSERT_NEAR(data2[1], 5.0, requiredPrecision);
  ASSERT_NEAR(data2[2], 6.0, requiredPrecision);
}

TEST(DataPoint, Access) {
  auto data = devices::data::DataPoint(1000, {1.0, 2.0, 3.0});
  ASSERT_EQ(data.getTimestamp(), 1000);
  ASSERT_EQ(data.size(), 3);
  ASSERT_NEAR(data[0], 1.0, requiredPrecision);
  ASSERT_NEAR(data[1], 2.0, requiredPrecision);
  ASSERT_NEAR(data[2], 3.0, requiredPrecision);

  // getData should return a const reference to the data, but we can use a const
  // cast to modify it
  { const_cast<double &>(data.getData()[0]) = 4.0; }

  // The value should have changed in the original data
  ASSERT_NEAR(data.getData()[0], 4.0, requiredPrecision);
}

TEST(DataPoint, Copy) {
  auto data = devices::data::DataPoint(1000, {1.0, 2.0, 3.0});

  // Copy should be similar to the original
  auto copy = data.copy();
  ASSERT_EQ(copy.getTimestamp(), 1000);
  ASSERT_EQ(copy.size(), 3);
  ASSERT_NEAR(copy[0], 1.0, requiredPrecision);
  ASSERT_NEAR(copy[1], 2.0, requiredPrecision);
  ASSERT_NEAR(copy[2], 3.0, requiredPrecision);

  // But should be a deep copy
  { const_cast<double &>(copy.getData()[0]) = 4.0; }
  ASSERT_NEAR(data[0], 1.0, requiredPrecision);
  ASSERT_NEAR(copy[0], 4.0, requiredPrecision);
}

TEST(DataPoint, serialize) {
  auto data = devices::data::DataPoint(1000, {1.0, 2.0, 3.0});
  auto json = data.serialize();
  ASSERT_EQ(json["timestamp"], 1000);
  ASSERT_EQ(json["data"].size(), 3);
  ASSERT_NEAR(json["data"][0], 1.0, requiredPrecision);
  ASSERT_NEAR(json["data"][1], 2.0, requiredPrecision);
  ASSERT_NEAR(json["data"][2], 3.0, requiredPrecision);
  ASSERT_STREQ(json.dump().c_str(),
               "{\"data\":[1.0,2.0,3.0],\"timestamp\":1000}");
}

TEST(DataPoint, deserialize) {
  nlohmann::json json = R"({"data":[1.0,2.0,3.0],"timestamp":1000})"_json;
  auto data = devices::data::DataPoint::deserialize(json);
  ASSERT_EQ(data.getTimestamp(), 1000);
  ASSERT_EQ(data.size(), 3);
  ASSERT_NEAR(data[0], 1.0, requiredPrecision);
  ASSERT_NEAR(data[1], 2.0, requiredPrecision);
  ASSERT_NEAR(data[2], 3.0, requiredPrecision);
}

TEST(Data, acquire) {
  //   auto devices = devices::data::DataDevices();
  //   devices.newDevice("Dummy");
  //   std::this_thread::sleep_for(std::chrono::milliseconds(10));

  //   auto callback = [&devices](const devices::data::DataPoint &newData) {
  //     devices["Dummy"].add(newData);
  //   };

  //   // TODO Redo the Mock
  //   auto nidaq = devices::NidaqDevice(4, 1000);
  //   nidaq.onNewData.listen(callback);
  //   nidaq.connect();
  //   nidaq.startRecording();
  //   std::this_thread::sleep_for(std::chrono::milliseconds(1));

  //   ASSERT_EQ(devices["Dummy"].size(), 1);
  //   const auto &dataFirstDeviceAfter = devices["Dummy"];
  //   ASSERT_GE(dataFirstDeviceAfter.size(), 1);
}