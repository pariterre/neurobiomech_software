#include <gtest/gtest.h>
#include <iostream>

#include "Data/FixedTimeSeries.h"
#include "Data/TimeSeries.h"
#include "Devices/DevicesData.h"

static double requiredPrecision(1e-10);

using namespace STIMWALKER_NAMESPACE;

TEST(DataPoint, Constructors) {
  auto now = std::chrono::system_clock::now();
  auto data = data::DataPoint({1.0, 2.0, 3.0});
  ASSERT_GE(data.getTimestamp().time_since_epoch().count(),
            now.time_since_epoch().count());
  ASSERT_LE(data.getTimestamp().time_since_epoch().count(),
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                now.time_since_epoch() + std::chrono::milliseconds(10))
                .count());

  ASSERT_EQ(data.size(), 3);
  ASSERT_NEAR(data[0], 1.0, requiredPrecision);
  ASSERT_NEAR(data[1], 2.0, requiredPrecision);
  ASSERT_NEAR(data[2], 3.0, requiredPrecision);

  auto data2 = data::DataPoint(std::chrono::milliseconds(100), {4.0, 5.0, 6.0});
  ASSERT_EQ(data2.getTimestamp(), std::chrono::system_clock::time_point(
                                      std::chrono::milliseconds(100)));
  ASSERT_EQ(data2.size(), 3);
  ASSERT_NEAR(data2[0], 4.0, requiredPrecision);
  ASSERT_NEAR(data2[1], 5.0, requiredPrecision);
  ASSERT_NEAR(data2[2], 6.0, requiredPrecision);
}

TEST(DataPoint, Access) {
  auto data = data::DataPoint(std::chrono::milliseconds(100), {1.0, 2.0, 3.0});
  ASSERT_EQ(data.getTimestamp(), std::chrono::system_clock::time_point(
                                     std::chrono::milliseconds(100)));
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
  auto data = data::DataPoint(std::chrono::milliseconds(100), {1.0, 2.0, 3.0});

  // Copy should be similar to the original
  auto copy = data.copy();
  ASSERT_EQ(copy.getTimestamp(), std::chrono::system_clock::time_point(
                                     std::chrono::milliseconds(100)));
  ASSERT_EQ(copy.size(), 3);
  ASSERT_NEAR(copy[0], 1.0, requiredPrecision);
  ASSERT_NEAR(copy[1], 2.0, requiredPrecision);
  ASSERT_NEAR(copy[2], 3.0, requiredPrecision);

  // But should be a deep copy
  { const_cast<double &>(copy.getData()[0]) = 4.0; }
  ASSERT_NEAR(data[0], 1.0, requiredPrecision);
  ASSERT_NEAR(copy[0], 4.0, requiredPrecision);
}

TEST(DataPoint, Serialize) {
  auto data = data::DataPoint(std::chrono::milliseconds(100), {1.0, 2.0, 3.0});
  auto json = data.serialize();
  ASSERT_EQ(json["timestamp"], 100 * 1000);
  ASSERT_EQ(json["data"].size(), 3);
  ASSERT_NEAR(json["data"][0], 1.0, requiredPrecision);
  ASSERT_NEAR(json["data"][1], 2.0, requiredPrecision);
  ASSERT_NEAR(json["data"][2], 3.0, requiredPrecision);
  ASSERT_STREQ(json.dump().c_str(),
               "{\"data\":[1.0,2.0,3.0],\"timestamp\":100000}");
}

TEST(DataPoint, Deserialize) {
  nlohmann::json json = R"({"data":[1.0,2.0,3.0],"timestamp":100000})"_json;
  auto data = data::DataPoint::deserialize(json);
  ASSERT_EQ(data.getTimestamp(), std::chrono::system_clock::time_point(
                                     std::chrono::milliseconds(100)));
  ASSERT_EQ(data.size(), 3);
  ASSERT_NEAR(data[0], 1.0, requiredPrecision);
  ASSERT_NEAR(data[1], 2.0, requiredPrecision);
  ASSERT_NEAR(data[2], 3.0, requiredPrecision);
}

TEST(TimeSeries, Access) {
  auto data = data::TimeSeries();

  // The data should be empty at the beginning
  ASSERT_EQ(data.size(), 0);

  // Add a five new data point
  data.add(data::DataPoint(std::chrono::milliseconds(100), {1.0, 2.0, 3.0}));
  data.add(data::DataPoint(std::chrono::milliseconds(200), {4.0, 5.0, 6.0}));
  data.add(data::DataPoint(std::chrono::milliseconds(300), {7.0, 8.0, 9.0}));
  data.add(data::DataPoint(std::chrono::milliseconds(400), {10.0, 11.0, 12.0}));
  auto now = std::chrono::system_clock::now();
  data.add(data::DataPoint({13.0, 14.0, 15.0}));

  // The size should be 5
  ASSERT_EQ(data.size(), 5);

  // The data should be accessible and correct
  ASSERT_EQ(data[0].getTimestamp(), std::chrono::system_clock::time_point(
                                        std::chrono::milliseconds(100)));
  ASSERT_EQ(data[0].size(), 3);
  ASSERT_NEAR(data[0][0], 1.0, requiredPrecision);
  ASSERT_NEAR(data[0][1], 2.0, requiredPrecision);
  ASSERT_NEAR(data[0][2], 3.0, requiredPrecision);
  ASSERT_EQ(data[1].getTimestamp(), std::chrono::system_clock::time_point(
                                        std::chrono::milliseconds(200)));
  ASSERT_GE(data[4].getTimestamp().time_since_epoch().count(),
            now.time_since_epoch().count());
  ASSERT_LE(data[4].getTimestamp().time_since_epoch().count(),
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                now.time_since_epoch() + std::chrono::milliseconds(10))
                .count());

  // Getting the data using the [] operator should return a const reference
  // To make sure, let's const cast and modify the data
  { const_cast<double &>(data[0].getData()[0]) = 100.0; }
  ASSERT_NEAR(data[0][0], 100.0, requiredPrecision);

  // Same for getData
  { const_cast<double &>(data.getData()[0].getData()[1]) = 200.0; }
  ASSERT_NEAR(data[0][1], 200.0, requiredPrecision);

  // Clearing the data should make it empty
  data.clear();
  ASSERT_EQ(data.size(), 0);
}

TEST(TimeSeries, NewData) {

  auto data = data::TimeSeries();
  data.add(data::DataPoint(std::chrono::milliseconds(100), {1.0, 2.0, 3.0}));
  auto now = std::chrono::system_clock::now();
  data.add(data::DataPoint({4.0, 5.0, 6.0}));

  // The first data point should have the timestamp set to 100, the second
  // should have the timestamp set to now or slightly after now
  ASSERT_EQ(data[0].getTimestamp(), std::chrono::system_clock::time_point(
                                        std::chrono::milliseconds(100)));
  ASSERT_GE(data[1].getTimestamp().time_since_epoch().count(),
            now.time_since_epoch().count());
  ASSERT_LE(data[1].getTimestamp().time_since_epoch().count(),
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                now.time_since_epoch() + std::chrono::milliseconds(10))
                .count());
}

TEST(TimeSeries, Serialize) {
  auto data = data::TimeSeries();
  data.add(data::DataPoint(std::chrono::milliseconds(100), {1.0, 2.0, 3.0}));
  data.add(data::DataPoint(std::chrono::milliseconds(200), {4.0, 5.0, 6.0}));
  data.add(data::DataPoint(std::chrono::milliseconds(300), {7.0, 8.0, 9.0}));
  data.add(data::DataPoint(std::chrono::milliseconds(400), {10.0, 11.0, 12.0}));
  data.add(data::DataPoint(std::chrono::milliseconds(500), {13.0, 14.0, 15.0}));

  auto json = data.serialize();
  ASSERT_EQ(json.size(), 5);
  ASSERT_EQ(json[0]["timestamp"], 100 * 1000);
  ASSERT_EQ(json[0]["data"].size(), 3);
  ASSERT_NEAR(json[0]["data"][0], 1.0, requiredPrecision);
  ASSERT_NEAR(json[0]["data"][1], 2.0, requiredPrecision);
  ASSERT_NEAR(json[0]["data"][2], 3.0, requiredPrecision);
  ASSERT_STREQ(
      json.dump().c_str(),
      "[{\"data\":[1.0,2.0,3.0],\"timestamp\":100000},{\"data\":[4.0,5."
      "0,6.0],\"timestamp\":200000},{\"data\":[7.0,8.0,9.0],"
      "\"timestamp\":300000},{\"data\":[10.0,11.0,12.0],\"timestamp\":"
      "400000},{\"data\":[13.0,14.0,15.0],\"timestamp\":500000}]");
}

TEST(TimeSeries, Deserialize) {
  nlohmann::json json =
      R"([{"data":[1.0,2.0,3.0],"timestamp":100000},{"data":[4.0,5.0,6.0],"timestamp":200000},{"data":[7.0,8.0,9.0],"timestamp":300000},{"data":[10.0,11.0,12.0],"timestamp":400000},{"data":[13.0,14.0,15.0],"timestamp":500000}])"_json;
  auto data = data::TimeSeries::deserialize(json);
  ASSERT_EQ(data.size(), 5);
  ASSERT_EQ(data[0].getTimestamp(), std::chrono::system_clock::time_point(
                                        std::chrono::milliseconds(100)));
  ASSERT_EQ(data[0].size(), 3);
  ASSERT_NEAR(data[0][0], 1.0, requiredPrecision);
  ASSERT_NEAR(data[0][1], 2.0, requiredPrecision);
  ASSERT_NEAR(data[0][2], 3.0, requiredPrecision);
  ASSERT_EQ(data[1].getTimestamp(), std::chrono::system_clock::time_point(
                                        std::chrono::milliseconds(200)));
}

TEST(FixedTimeSeries, NewData) {
  // TODO ADD ALL THE CONSTRUCTOR TESTS AND MAKE SURE CHANGING THE STARTING TIME
  // WORKS

  auto data = data::FixedTimeSeries(std::chrono::milliseconds(100));
  ASSERT_EQ(data.getDeltaTime(), std::chrono::milliseconds(100));

  EXPECT_THROW(data.add(data::DataPoint(std::chrono::milliseconds(100),
                                        {1.0, 2.0, 3.0})),
               std::runtime_error);
  data.add({1.0, 2.0, 3.0});
  data.add({4.0, 5.0, 6.0});

  // The first data point should be 500, but the second should be 600 (500 +
  // 100) even if we set the timestamp to 500 as FixedTimeSeries should add the
  // DeltaTime to the previous timestamp if it is not the first data point
  ASSERT_EQ(data[0].getTimestamp(), std::chrono::system_clock::time_point(
                                        std::chrono::milliseconds(500)));
  ASSERT_EQ(data[1].getTimestamp(),
            std::chrono::system_clock::time_point(
                std::chrono::milliseconds(500) + data.getDeltaTime()));

  // Clearing the data should make it empty, therefore reset the DeltaTime rule
  data.clear();
  data.add({1.0, 2.0, 3.0});
  data.add({4.0, 5.0, 6.0});
  ASSERT_EQ(data[0].getTimestamp(), std::chrono::system_clock::time_point(
                                        std::chrono::milliseconds(500)));
  ASSERT_EQ(data[1].getTimestamp(),
            std::chrono::system_clock::time_point(
                std::chrono::milliseconds(500) + data.getDeltaTime()));

  // Not sending any value for the time stamp for the first data point should
  // set it to the current time
  data.clear();
  auto now = std::chrono::system_clock::now();
  data.add(data::DataPoint({1.0, 2.0, 3.0}));
  data.add(data::DataPoint(std::chrono::milliseconds(500), {4.0, 5.0, 6.0}));

  ASSERT_GE(data[0].getTimestamp().time_since_epoch().count(),
            now.time_since_epoch().count());
  ASSERT_LE(data[0].getTimestamp().time_since_epoch().count(),
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                now.time_since_epoch() + std::chrono::milliseconds(10))
                .count());
  ASSERT_GE(data[1].getTimestamp().time_since_epoch().count(),
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                now.time_since_epoch() + data.getDeltaTime())
                .count());
  ASSERT_LE(data[1].getTimestamp().time_since_epoch().count(),
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                now.time_since_epoch() + data.getDeltaTime() +
                std::chrono::milliseconds(10))
                .count());
}