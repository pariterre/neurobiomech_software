#include <Devices/DelsysEmgDevice.h>
#include <Utils/Logger.h>

#include <chrono>
#include <thread>

using namespace STIMWALKER_NAMESPACE;

int main() {
  auto &logger = utils::Logger::getInstance();
  logger.setLogLevel(utils::Logger::INFO);

  logger.info("Starting the application");

  try {
    // asio::io_context context;
    // asio::ip::tcp::socket socket(context);
    // asio::ip::tcp::endpoint endpoint(
    //     asio::ip::address::from_string("127.0.0.1"), 50040);
    // std::vector<char> buffer(128, 0);

    // asio::ip::tcp::socket dataSocket(context);
    // asio::ip::tcp::endpoint dataEndpoint(
    //     asio::ip::address::from_string("127.0.0.1"), 50043);
    // std::vector<char> dataBuffer(2048, 0);

    // socket.connect(endpoint);
    // dataSocket.connect(dataEndpoint);
    // socket.receive(asio::buffer(buffer));
    // std::cout << std::string(buffer.begin(), buffer.end()).c_str() <<
    // std::endl;

    // // Send a START command to the device
    // std::cout << "Sending START command" << std::endl;
    // std::string startCommand = "START\r\n\r\n";
    // socket.send(asio::buffer(startCommand));
    // socket.receive(asio::buffer(buffer));
    // if (std::strncmp(buffer.data(), "OK", 2) != 0) {
    //   throw std::runtime_error("Command failed: START");
    // }
    // std::cout << std::string(buffer.begin(), buffer.end()).c_str() <<
    // std::endl;

    // // Wait for some time
    // std::vector<std::vector<float>> allData;
    // while (allData.size() < 1000) {
    //   int bytesPerChannel = 4;
    //   int channelCount = 16;
    //   int sampleCount = 27; // TODO Find out why 27...

    //   dataSocket.receive(asio::buffer(dataBuffer));
    //   std::vector<float> data(channelCount * sampleCount);
    //   std::memcpy(data.data(), dataBuffer.data(),
    //               channelCount * sampleCount * bytesPerChannel);
    //   for (int i = 0; i < sampleCount; i++) {
    //     allData.push_back(
    //         std::vector<float>(data.begin() + i * channelCount,
    //                            data.begin() + (i + 1) * channelCount));
    //   }
    // }

    // // Send a STOP command to the device
    // std::cout << "Sending STOP command" << std::endl;
    // std::string stopCommand = "STOP\r\n\r\n";
    // socket.send(asio::buffer(stopCommand));

    // for (auto &data : allData) {
    //   std::cout << data[0] << "," << std::endl;
    // }

    auto delsys = devices::DelsysEmgDeviceMock();

    delsys.connect();
    logger.info("Connected");

    delsys.startRecording();
    delsys.readData();

    std::this_thread::sleep_for(std::chrono::seconds(5));
    // // Get the data
    // logger.info("Get last values");

    delsys.disconnect();
    logger.info("Disconnected");
  } catch (std::exception &e) {
    logger.fatal(e.what());
    return EXIT_FAILURE;
  }

  logger.info("Exiting the application");
  return EXIT_SUCCESS;
}