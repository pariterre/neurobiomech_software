#include <gtest/gtest.h>
#include <iostream>

#include "Data/FixedTimeSeries.h"
#include "Data/TimeSeries.h"

static double requiredPrecision(1e-10);

using namespace STIMWALKER_NAMESPACE;
void ASSERT_ALMOST_NOW(const std::chrono::system_clock::time_point &time,
                       const std::chrono::system_clock::time_point &now) {
  auto timeCount = std::chrono::duration_cast<std::chrono::nanoseconds>(
                       time.time_since_epoch())
                       .count();
  auto nowCount = std::chrono::duration_cast<std::chrono::nanoseconds>(
                      now.time_since_epoch())
                      .count();
  auto nowDelayCount = std::chrono::duration_cast<std::chrono::nanoseconds>(
                           std::chrono::milliseconds(50))
                           .count();
  ASSERT_GE(timeCount, nowCount);
  ASSERT_LE(timeCount, nowCount + nowDelayCount);
}

TEST(DataPoint, Constructors) {
  auto now = std::chrono::system_clock::now();

  {
    auto data = data::DataPoint({1.0, 2.0, 3.0});
    ASSERT_EQ(data.getTimestamp(), std::chrono::microseconds(-1));

    ASSERT_EQ(data.size(), 3);
    ASSERT_NEAR(data[0], 1.0, requiredPrecision);
    ASSERT_NEAR(data[1], 2.0, requiredPrecision);
    ASSERT_NEAR(data[2], 3.0, requiredPrecision);
  }

  {
    auto data =
        data::DataPoint(std::chrono::milliseconds(100), {4.0, 5.0, 6.0});
    ASSERT_EQ(data.getTimestamp(), std::chrono::milliseconds(100));
    ASSERT_EQ(data.size(), 3);
    ASSERT_NEAR(data[0], 4.0, requiredPrecision);
    ASSERT_NEAR(data[1], 5.0, requiredPrecision);
    ASSERT_NEAR(data[2], 6.0, requiredPrecision);
  }
}

TEST(DataPoint, Access) {
  auto data = data::DataPoint(std::chrono::milliseconds(100), {1.0, 2.0, 3.0});
  ASSERT_EQ(data.getTimestamp(), std::chrono::milliseconds(100));
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
  ASSERT_EQ(copy.getTimestamp(), std::chrono::milliseconds(100));
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
  ASSERT_EQ(data.getTimestamp(), std::chrono::milliseconds(100));
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
  ASSERT_EQ(data[0].getTimestamp(), std::chrono::milliseconds(100));
  ASSERT_EQ(data[0].size(), 3);
  ASSERT_NEAR(data[0][0], 1.0, requiredPrecision);
  ASSERT_NEAR(data[0][1], 2.0, requiredPrecision);
  ASSERT_NEAR(data[0][2], 3.0, requiredPrecision);
  ASSERT_EQ(data[1].getTimestamp(), std::chrono::milliseconds(200));
  ASSERT_GE(data[4].getTimestamp(), std::chrono::microseconds(0));
  ASSERT_LE(data[4].getTimestamp(), std::chrono::microseconds(100));

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
  ASSERT_EQ(data[0].getTimestamp(), std::chrono::milliseconds(100));
  ASSERT_EQ(data[1].getTimestamp(), std::chrono::microseconds(-1));
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
  ASSERT_EQ(data[0].getTimestamp(), std::chrono::milliseconds(100));
  ASSERT_EQ(data[0].size(), 3);
  ASSERT_NEAR(data[0][0], 1.0, requiredPrecision);
  ASSERT_NEAR(data[0][1], 2.0, requiredPrecision);
  ASSERT_NEAR(data[0][2], 3.0, requiredPrecision);
  ASSERT_EQ(data[1].getTimestamp(), std::chrono::milliseconds(200));
}

TEST(FixedTimeSeries, Constructors) {
  // Testing the constructor that uses now as the starting time
  {
    auto now = std::chrono::system_clock::now();
    auto data = data::FixedTimeSeries(std::chrono::microseconds(100));
    ASSERT_EQ(data.getDeltaTime(), std::chrono::microseconds(100));

    data.add(data::DataPoint({1.0, 2.0, 3.0}));
    data.add(data::DataPoint({4.0, 5.0, 6.0}));
    ASSERT_ALMOST_NOW(data.getStartingTime(), now);
    ASSERT_EQ(data[0].getTimestamp(), std::chrono::microseconds(0));
    ASSERT_EQ(data[1].getTimestamp(),
              std::chrono::microseconds(0) + data.getDeltaTime());
  }
  {
    auto now = std::chrono::system_clock::now();
    auto data = data::FixedTimeSeries(std::chrono::milliseconds(100));
    ASSERT_EQ(data.getDeltaTime(), std::chrono::milliseconds(100));

    data.add(data::DataPoint({1.0, 2.0, 3.0}));
    data.add(data::DataPoint({4.0, 5.0, 6.0}));
    ASSERT_ALMOST_NOW(data.getStartingTime(), now);
    ASSERT_EQ(data[0].getTimestamp(), std::chrono::microseconds(0));
    ASSERT_EQ(data[1].getTimestamp(),
              std::chrono::microseconds(0) + data.getDeltaTime());
  }

  // Testing the constructor that specify a starting time
  {
    auto data = data::FixedTimeSeries(
        std::chrono::system_clock::time_point(std::chrono::microseconds(300)),
        std::chrono::microseconds(100));
    ASSERT_EQ(data.getDeltaTime(), std::chrono::microseconds(100));

    data.add(data::DataPoint({1.0, 2.0, 3.0}));
    data.add(data::DataPoint({4.0, 5.0, 6.0}));
    ASSERT_EQ(std::chrono::duration_cast<std::chrono::microseconds>(
                  data.getStartingTime().time_since_epoch()),
              std::chrono::microseconds(300));
    ASSERT_EQ(data[0].getTimestamp(), std::chrono::microseconds(0));
    ASSERT_EQ(data[1].getTimestamp(), std::chrono::microseconds(0 + 100));
  }
}

TEST(FixedTimeSeries, NewData) {
  // Test the setters
  {
    auto now = std::chrono::system_clock::now();
    auto data = data::FixedTimeSeries(std::chrono::milliseconds(100));
    // Wait for a bit to make sure the time is different
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_ALMOST_NOW(data.getStartingTime(), now);

    data.reset();
    ASSERT_GT(data.getStartingTime(), now);
  }

  // Test that setting the time before adding data works properly, using
  // non-timed data
  {
    auto now = std::chrono::system_clock::now();
    auto data = data::FixedTimeSeries(std::chrono::milliseconds(100));

    data.add(data::DataPoint({1.0, 2.0, 3.0}));
    data.add(data::DataPoint({4.0, 5.0, 6.0}));
    ASSERT_ALMOST_NOW(data.getStartingTime(), now);

    // Clearing should reset data, but not the timer
    data.clear();
    ASSERT_ALMOST_NOW(data.getStartingTime(), now);
    ASSERT_EQ(data.size(), 0);

    // Reset should clear the data and reset the timer
    data.add(data::DataPoint({1.0, 2.0, 3.0}));
    data.add(data::DataPoint({4.0, 5.0, 6.0}));
    data.reset();
    ASSERT_GT(data.getStartingTime(), now);
    ASSERT_EQ(data.size(), 0);
  }
}
