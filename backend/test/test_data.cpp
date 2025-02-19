#include <gtest/gtest.h>
#include <iostream>
#include <thread>

#include "Data/FixedTimeSeries.h"
#include "Data/TimeSeries.h"

#include "utils.h"

static double requiredPrecision(1e-10);

using namespace NEUROBIO_NAMESPACE;

TEST(DataPoint, Constructors) {
  auto now = std::chrono::system_clock::now();

  {
    auto data = data::DataPoint(std::chrono::microseconds(10), {1.0, 2.0, 3.0});

    ASSERT_EQ(data.size(), 3);
    ASSERT_EQ(data.getTimeStamp(), std::chrono::microseconds(10));
    ASSERT_NEAR(data[0], 1.0, requiredPrecision);
    ASSERT_NEAR(data[1], 2.0, requiredPrecision);
    ASSERT_NEAR(data[2], 3.0, requiredPrecision);
  }
}

TEST(DataPoint, Access) {
  auto data = data::DataPoint(std::chrono::microseconds(10), {1.0, 2.0, 3.0});

  // Accessing the data should throw an exception if the index is out of range
  ASSERT_THROW(data[3], std::out_of_range);

  // The [getTimeStamp] method should return a (const) reference to the
  // timestamp, make sure by changing it (using a const_cast)
  {
    const_cast<std::chrono::microseconds &>(data.getTimeStamp()) =
        std::chrono::microseconds(100);
  }
  ASSERT_EQ(data.getTimeStamp(), std::chrono::microseconds(100));

  // The [getData] method should return a (const) reference to the data, make
  // sure by changing it (using a const_cast)
  {
    const_cast<double &>(data.getData()[0]) = 4.0;
  }
  ASSERT_NEAR(data[0], 4.0, requiredPrecision);
}

TEST(DataPoint, Serialize) {
  auto data = data::DataPoint(std::chrono::microseconds(10), {1.0, 2.0, 3.0});
  auto json = data.serialize();
  ASSERT_EQ(json.size(), 2);
  ASSERT_EQ(json[0], 10);
  ASSERT_EQ(json[1].size(), 3);
  ASSERT_NEAR(json[1][0], 1.0, requiredPrecision);
  ASSERT_NEAR(json[1][1], 2.0, requiredPrecision);
  ASSERT_NEAR(json[1][2], 3.0, requiredPrecision);
  ASSERT_STREQ(json.dump().c_str(), "[10,[1.0,2.0,3.0]]");
}

TEST(DataPoint, Deserialize) {
  nlohmann::json json = R"([10,[1.0,2.0,3.0]])"_json;
  auto data = data::DataPoint(json);
  ASSERT_EQ(data.getTimeStamp(), std::chrono::microseconds(10));
  ASSERT_EQ(data.size(), 3);
  ASSERT_NEAR(data[0], 1.0, requiredPrecision);
  ASSERT_NEAR(data[1], 2.0, requiredPrecision);
  ASSERT_NEAR(data[2], 3.0, requiredPrecision);
}

TEST(TimeSeries, StartingTime) {
  auto now = std::chrono::system_clock::now();
  auto data = data::TimeSeries();

  // Wait longer than the ASSERT_ALMOST_NOW threshold sure the current time is
  // different
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // The starting time should be set to now
  ASSERT_ALMOST_NOW(data.getStartingTime(), now);

  // The starting time should be accessible and correct
  ASSERT_EQ(data.getStartingTime(), data.getStartingTime());
  ASSERT_EQ(data.getStartingTime(), data.getStartingTime());

  // Clearing the data should not change the starting time
  data.clear();
  ASSERT_ALMOST_NOW(data.getStartingTime(), now);

  // Resetting the data should change the starting time
  now = std::chrono::system_clock::now();
  data.reset();
  ASSERT_ALMOST_NOW(data.getStartingTime(), now);
}

TEST(TimeSeries, AccessData) {
  auto data = data::TimeSeries();

  // The data should be empty at the beginning
  ASSERT_EQ(data.size(), 0);

  // Add a five new data point (includes one with no given timestamp)
  data.add(std::chrono::milliseconds(100), {1.0, 2.0, 3.0});
  data.add(std::chrono::milliseconds(200), {4.0, 5.0, 6.0});
  data.add(std::chrono::milliseconds(300), {7.0, 8.0, 9.0});
  data.add(std::chrono::milliseconds(400), {10.0, 11.0, 12.0});
  data.add({13.0, 14.0, 15.0});

  // The size should be 5
  ASSERT_EQ(data.size(), 5);

  // Accessing the data should throw an exception if the index is out of range
  ASSERT_THROW(data[5], std::out_of_range);

  // The timestamps should be accessible and correct
  ASSERT_EQ(&data.front(), &data.getData()[0]);
  ASSERT_EQ(&data.back(), &data.getData()[4]);

  ASSERT_EQ(&data[0], &data.getData()[0]);
  ASSERT_EQ(&data[4], &data.getData()[4]);

  ASSERT_EQ(data.getData()[0].getTimeStamp(), std::chrono::milliseconds(100));
  ASSERT_EQ(data.getData()[1].getTimeStamp(), std::chrono::milliseconds(200));
  // The non given timestamp is somewhere between 0 and the time it took to get
  // from the declaration of TimeSeries and the add of the 5th point. So let's
  // assume it is between 0 and 50
  ASSERT_GE(data.getData()[4].getTimeStamp(), std::chrono::milliseconds(0));
  ASSERT_LE(data.getData()[4].getTimeStamp(), std::chrono::milliseconds(50));

  // The data should be accessible and correct
  ASSERT_EQ(data[0].getData().size(), 3);
  ASSERT_NEAR(data[0].getData()[0], 1.0, requiredPrecision);
  ASSERT_NEAR(data[0].getData()[1], 2.0, requiredPrecision);
  ASSERT_NEAR(data[0].getData()[2], 3.0, requiredPrecision);
  ASSERT_EQ(data[1].getData().size(), 3);
  ASSERT_NEAR(data[1].getData()[0], 4.0, requiredPrecision);
  ASSERT_NEAR(data[1].getData()[1], 5.0, requiredPrecision);
  ASSERT_NEAR(data[1].getData()[2], 6.0, requiredPrecision);

  // Getting the data using the [] operator should return a (const) reference to
  // the timestamp and data, make sure by changing it (using a const_cast)
  {
    const_cast<std::chrono::microseconds &>(data[0].getTimeStamp()) =
        std::chrono::microseconds(1000);
  }
  {
    const_cast<double &>(data[0].getData()[0]) = 100.0;
  }

  ASSERT_EQ(data[0].getTimeStamp(), std::chrono::microseconds(1000));
  ASSERT_NEAR(data[0].getData()[0], 100.0, requiredPrecision);

  // Same for getData
  {
    const_cast<double &>(data[0].getData()[1]) = 200.0;
  }
  ASSERT_NEAR(data[0].getData()[1], 200.0, requiredPrecision);

  // Getting a slice of the data should return the data between the two indices
  auto slice = data.slice(1, 3);
  ASSERT_EQ(slice.getStartingTime(), data.getStartingTime());
  ASSERT_EQ(slice.size(), 2);
  ASSERT_EQ(slice[0].getTimeStamp(), std::chrono::milliseconds(200));
  ASSERT_EQ(slice[0].getData().size(), 3);
  ASSERT_NEAR(slice[0].getData()[0], 4.0, requiredPrecision);
  ASSERT_NEAR(slice[0].getData()[1], 5.0, requiredPrecision);
  ASSERT_NEAR(slice[0].getData()[2], 6.0, requiredPrecision);
  ASSERT_EQ(slice[1].getTimeStamp(), std::chrono::milliseconds(300));

  // Getting the last n data should return the last n data
  auto tail = data.tail(3);
  ASSERT_EQ(tail.getStartingTime(), data.getStartingTime());
  ASSERT_EQ(tail.size(), 3);
  ASSERT_EQ(tail[0].getTimeStamp(), std::chrono::milliseconds(300));
  ASSERT_EQ(tail[0].getData().size(), 3);
  ASSERT_NEAR(tail[0].getData()[0], 7.0, requiredPrecision);
  ASSERT_NEAR(tail[0].getData()[1], 8.0, requiredPrecision);
  ASSERT_NEAR(tail[0].getData()[2], 9.0, requiredPrecision);
  ASSERT_EQ(tail[1].getTimeStamp(), std::chrono::milliseconds(400));

  // Getting the data since a specific time should return the data since that
  // time
  auto since =
      data.since(data.getStartingTime() + std::chrono::milliseconds(300));
  ASSERT_EQ(since.getStartingTime(), data.getStartingTime());
  ASSERT_EQ(since.size(), 2); // data[4] is actually before 300
  ASSERT_EQ(since[0].getTimeStamp(), std::chrono::milliseconds(300));
  ASSERT_EQ(since[0].getData().size(), 3);
  ASSERT_NEAR(since[0].getData()[0], 7.0, requiredPrecision);
  ASSERT_NEAR(since[0].getData()[1], 8.0, requiredPrecision);
  ASSERT_NEAR(since[0].getData()[2], 9.0, requiredPrecision);
  ASSERT_EQ(since[1].getTimeStamp(), std::chrono::milliseconds(400));
}

TEST(TimeSeries, ClearingData) {

  auto data = data::TimeSeries();
  data.add(std::chrono::milliseconds(100), {1.0, 2.0, 3.0});
  data.add({4.0, 5.0, 6.0});

  // The first data point should have the timestamp set to 100, the second
  // should have the timestamp slightly over 0
  ASSERT_EQ(data.size(), 2);

  // Clearing the data should remove all the data
  data.clear();
  ASSERT_EQ(data.size(), 0);

  // Adding new data should work
  data.add(std::chrono::milliseconds(200), {7.0, 8.0, 9.0});
  ASSERT_EQ(data.size(), 1);

  // Resetting should also remove all the data
  data.reset();
  ASSERT_EQ(data.size(), 0);
}

TEST(TimeSeries, Serialize) {
  auto data = data::TimeSeries(
      std::chrono::system_clock::time_point(std::chrono::seconds(100)));
  data.add(std::chrono::milliseconds(100), {1.0, 2.0, 3.0});
  data.add(std::chrono::milliseconds(200), {4.0, 5.0, 6.0});
  data.add(std::chrono::milliseconds(300), {7.0, 8.0, 9.0});
  data.add(std::chrono::milliseconds(400), {10.0, 11.0, 12.0});
  data.add(std::chrono::milliseconds(500), {13.0, 14.0, 15.0});

  auto json = data.serialize();
  ASSERT_EQ(json.size(), 2);
  ASSERT_EQ(json["starting_time"], 100 * 1000 * 1000);
  ASSERT_EQ(json["data"].size(), 5);
  ASSERT_EQ(json["data"][0][0], 100 * 1000);
  ASSERT_EQ(json["data"][0][1].size(), 3);
  ASSERT_NEAR(json["data"][0][1][0], 1.0, requiredPrecision);
  ASSERT_NEAR(json["data"][0][1][1], 2.0, requiredPrecision);
  ASSERT_NEAR(json["data"][0][1][2], 3.0, requiredPrecision);
  ASSERT_STREQ(json.dump().c_str(),
               "{\"data\":[[100000,[1.0,2.0,3.0]],[200000,[4.0,5.0,6.0]],["
               "300000,[7.0,8.0,9.0]],[400000,[10.0,11.0,12.0]],[500000,[13.0,"
               "14.0,15.0]]],\"startingTime\":100000000}");
}

TEST(TimeSeries, Deserialize) {
  nlohmann::json json =
      R"({"data":[[100000,[1.0,2.0,3.0]],[200000,[4.0,5.0,6.0]],[300000,[7.0,8.0,9.0]],[400000,[10.0,11.0,12.0]],[500000,[13.0,14.0,15.0]]],"startingTime":100000000})"_json;
  auto data = data::TimeSeries(json);
  ASSERT_ALMOST_NOW(
      data.getStartingTime(),
      std::chrono::system_clock::time_point(std::chrono::seconds(100)));
  ASSERT_EQ(data.size(), 5);
  ASSERT_EQ(data[0].getTimeStamp(), std::chrono::milliseconds(100));
  ASSERT_EQ(data[0].getData().size(), 3);
  ASSERT_NEAR(data[0].getData()[0], 1.0, requiredPrecision);
  ASSERT_NEAR(data[0].getData()[1], 2.0, requiredPrecision);
  ASSERT_NEAR(data[0].getData()[2], 3.0, requiredPrecision);
  ASSERT_EQ(data[1].getTimeStamp(), std::chrono::milliseconds(200));
  ASSERT_EQ(data[1].getData().size(), 3);
  ASSERT_NEAR(data[1].getData()[0], 4.0, requiredPrecision);
  ASSERT_NEAR(data[1].getData()[1], 5.0, requiredPrecision);
  ASSERT_NEAR(data[1].getData()[2], 6.0, requiredPrecision);
}

TEST(FixedTimeSeries, Constructors) {
  // Testing the constructor that uses now as the starting time
  {
    auto now = std::chrono::system_clock::now();
    auto data = data::FixedTimeSeries(std::chrono::microseconds(100));
    ASSERT_EQ(data.getDeltaTime(), std::chrono::microseconds(100));

    data.add({1.0, 2.0, 3.0});
    data.add({4.0, 5.0, 6.0});
    ASSERT_ALMOST_NOW(data.getStartingTime(), now);
    ASSERT_EQ(data[0].getTimeStamp(), std::chrono::microseconds(0));
    ASSERT_EQ(data[1].getTimeStamp(), std::chrono::microseconds(0 + 100));
  }
  {
    auto now = std::chrono::system_clock::now();
    auto data = data::FixedTimeSeries(std::chrono::milliseconds(100));
    ASSERT_EQ(data.getDeltaTime(), std::chrono::milliseconds(100));

    data.add({1.0, 2.0, 3.0});
    data.add({4.0, 5.0, 6.0});
    ASSERT_ALMOST_NOW(data.getStartingTime(), now);
    ASSERT_EQ(data[0].getTimeStamp(), std::chrono::microseconds(0));
    ASSERT_EQ(data[1].getTimeStamp(), std::chrono::milliseconds(0 + 100));
  }

  // Testing the constructor that specify a starting time
  {
    auto data = data::FixedTimeSeries(
        std::chrono::system_clock::time_point(std::chrono::microseconds(300)),
        std::chrono::microseconds(100));
    ASSERT_EQ(data.getDeltaTime(), std::chrono::microseconds(100));

    data.add({1.0, 2.0, 3.0});
    data.add({4.0, 5.0, 6.0});
    ASSERT_EQ(std::chrono::duration_cast<std::chrono::microseconds>(
                  data.getStartingTime().time_since_epoch()),
              std::chrono::microseconds(300));
    ASSERT_EQ(data[0].getTimeStamp(), std::chrono::microseconds(0));
    ASSERT_EQ(data[1].getTimeStamp(), std::chrono::microseconds(0 + 100));
  }
}
