#include <gtest/gtest.h>
#include <iostream>
#include <thread>

#include "Server/TcpClient.h"
#include "Server/TcpServer.h"
#include "utils.h"

static double requiredPrecision(1e-10);

std::chrono::milliseconds failingTimeoutPeriod(500);
std::chrono::milliseconds failingBufferPeriod(50);
#ifdef WIN32
std::chrono::milliseconds sufficientTimeoutPeriod(4000);
#else
std::chrono::milliseconds sufficientTimeoutPeriod(500);
#endif
void ensureServerIsConnected(
    const STIMWALKER_NAMESPACE::server::TcpServerMock &server) {
  auto startingWaitingTime = std::chrono::high_resolution_clock::now();
  while (true) {
    if (server.isClientConnected() ||
        std::chrono::high_resolution_clock::now() >
            startingWaitingTime + sufficientTimeoutPeriod) {
      break;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}
void ensureServerIsDisconnected(
    const STIMWALKER_NAMESPACE::server::TcpServerMock &server) {
  auto startingWaitingTime = std::chrono::high_resolution_clock::now();
  while (true) {
    if (!server.isClientConnected() ||
        std::chrono::high_resolution_clock::now() >
            startingWaitingTime + sufficientTimeoutPeriod) {
      break;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

using namespace STIMWALKER_NAMESPACE;

TEST(Server, Ports) {
  auto logger = TestLogger();

  {
    server::TcpServer server;
    ASSERT_EQ(server.getCommandPort(), 5000);
    ASSERT_EQ(server.getResponsePort(), 5001);
  }

  {
    server::TcpServer server(5010, 5011);
    ASSERT_EQ(server.getCommandPort(), 5010);
    ASSERT_EQ(server.getResponsePort(), 5011);
  }
}

TEST(Server, StartServer) {
  auto logger = TestLogger();

  server::TcpServer server;
  server.startServer();
  logger.giveTimeToUpdate();

  // Test the server actually started
  ASSERT_TRUE(logger.contains("Command server started on port 5000"));
  ASSERT_TRUE(logger.contains("Response server started on port 5001"));
  logger.clear();

  // Stop the server
  server.stopServer();

  // Test the server actually stopped
  ASSERT_TRUE(logger.contains(
      "Stopping listening to ports as server is shutting down"));
  ASSERT_TRUE(logger.contains("Server has shut down"));
}

#ifndef SKIP_LONG_TESTS
TEST(Server, ClientConnexion) {
  auto logger = TestLogger();

  {
    server::TcpServerMock server(5000, 5001, failingTimeoutPeriod);
    server.startServer();

    logger.giveTimeToUpdate();
    ASSERT_TRUE(logger.contains("Command server started on port 5000"));
    ASSERT_TRUE(logger.contains("Response server started on port 5001"));
    ASSERT_TRUE(logger.contains("Waiting for a new connexion"));

    // Server can be shut if nothing happened
    server.stopServer();
    ASSERT_TRUE(logger.contains("Server has shut down"));
  }
  logger.clear();

  {
    server::TcpServerMock server(5000, 5001, failingTimeoutPeriod);
    server.startServer();
    logger.giveTimeToUpdate();

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto socket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*socket, resolver.resolve("localhost", std::to_string(5000)));
    // Give some time to the message to arrive
    logger.giveTimeToUpdate();
    ASSERT_TRUE(logger.contains("Command socket connected to client, waiting "
                                "for a connexion to the response socket"));
  }
  // Server can be shut if one connexion was made and is waiting for the reponse
  // port
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if one connexion was made but dropped
  {
    server::TcpServerMock server(5000, 5001, failingTimeoutPeriod);
    server.startServer();

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto socket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*socket, resolver.resolve("localhost", std::to_string(5000)));

    // Wait longer than the timeout
    std::this_thread::sleep_for(failingTimeoutPeriod + failingBufferPeriod);
    ASSERT_TRUE(logger.contains("Response socket connection timeout (" +
                                std::to_string(failingTimeoutPeriod.count()) +
                                " ms), disconnecting client"));
  }
  ASSERT_TRUE("All devices are now disconnected");
  ASSERT_TRUE("Stopping listening to ports as server is shutting down");
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions to command and response ports were made,
  // but waiting for the handshake
  {
    server::TcpServerMock server(5000, 5001, sufficientTimeoutPeriod);
    server.startServer();

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto commandSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    auto responseSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*responseSocket,
                  resolver.resolve("localhost", std::to_string(5001)));

    // Give some time to the message to arrive
    logger.giveTimeToUpdate();
    ASSERT_TRUE(logger.contains(
        "Response socket connected to client, waiting for official handshake"));
  }
  ASSERT_TRUE(logger.contains("Disconnecting client"));
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions to command and response ports were made,
  // but waiting for the handshake timed out
  {
    server::TcpServerMock server(5000, 5001, sufficientTimeoutPeriod);
    server.startServer();

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto commandSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto responseSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    asio::connect(*responseSocket,
                  resolver.resolve("localhost", std::to_string(5001)));

    // Wait longer than the timeout
    ensureServerIsConnected(server);
    server.setTimeoutPeriod(failingTimeoutPeriod);
    std::this_thread::sleep_for(failingTimeoutPeriod + failingBufferPeriod);

    logger.giveTimeToUpdate();
    ASSERT_TRUE(logger.contains("Handshake timeout (" +
                                std::to_string(failingTimeoutPeriod.count()) +
                                +" ms), disconnecting client"));
    ASSERT_TRUE(logger.contains("Disconnecting client"));
    ASSERT_TRUE(logger.contains("All devices are now disconnected"));
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions to command and response ports were made,
  // but waiting for the handshake timed out, and a connexion was retried
  {
    server::TcpServerMock server(5000, 5001, sufficientTimeoutPeriod);
    server.startServer();

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto commandSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto responseSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    asio::connect(*responseSocket,
                  resolver.resolve("localhost", std::to_string(5001)));

    // Wait longer than the timeout
    ensureServerIsConnected(server);
    server.setTimeoutPeriod(failingTimeoutPeriod);
    std::this_thread::sleep_for(failingTimeoutPeriod + failingBufferPeriod);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));

    // Give some time to the message to arrive
    logger.giveTimeToUpdate();
    ASSERT_EQ(logger.count("Command socket connected to client, waiting for a "
                           "connexion to the response socket"),
              2);
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions to command and response ports were made,
  // but waiting for the handshake timed out, and a connexion was retried but
  // response socket connexion timed out
  {
    server::TcpServerMock server(5000, 5001, sufficientTimeoutPeriod);
    server.startServer();

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto commandSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto responseSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    asio::connect(*responseSocket,
                  resolver.resolve("localhost", std::to_string(5001)));

    // Wait longer than the timeout
    ensureServerIsConnected(server);
    server.setTimeoutPeriod(failingTimeoutPeriod);
    std::this_thread::sleep_for(failingTimeoutPeriod + failingBufferPeriod);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    std::this_thread::sleep_for(failingTimeoutPeriod + failingBufferPeriod);

    ASSERT_TRUE(logger.contains("Response socket connection timeout (" +
                                std::to_string(failingTimeoutPeriod.count()) +
                                " ms), "
                                "disconnecting client"));
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions to command and response ports were made,
  // but waiting for the handshake timed out, and a connexion was retried and
  // response socket connexion was made
  {
    server::TcpServerMock server(5000, 5001, sufficientTimeoutPeriod);
    server.startServer();

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto commandSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto responseSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    asio::connect(*responseSocket,
                  resolver.resolve("localhost", std::to_string(5001)));

    // Wait longer than the timeout
    ensureServerIsConnected(server);
    server.setTimeoutPeriod(failingTimeoutPeriod);
    std::this_thread::sleep_for(failingTimeoutPeriod + failingBufferPeriod);
    server.setTimeoutPeriod(sufficientTimeoutPeriod);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    asio::connect(*responseSocket,
                  resolver.resolve("localhost", std::to_string(5001)));
    ensureServerIsConnected(server);

    // Give some time to the message to arrive
    logger.giveTimeToUpdate();
    ASSERT_EQ(logger.count("Response socket connected to client, waiting for "
                           "official handshake"),
              2);
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions to command and response ports were made,
  // but waiting for the handshake timed out, and a connexion was retried and
  // response socket connexion was made, but handshaked timed out
  {
    server::TcpServerMock server(5000, 5001, sufficientTimeoutPeriod);
    server.startServer();

    asio::io_context context;
    asio::ip::tcp::resolver resolver(context);
    auto commandSocket = std::make_unique<asio::ip::tcp::socket>(context);
    auto responseSocket = std::make_unique<asio::ip::tcp::socket>(context);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    asio::connect(*responseSocket,
                  resolver.resolve("localhost", std::to_string(5001)));

    // Wait longer than the timeout
    ensureServerIsConnected(server);
    server.setTimeoutPeriod(failingTimeoutPeriod);
    std::this_thread::sleep_for(failingTimeoutPeriod + failingBufferPeriod);
    server.setTimeoutPeriod(sufficientTimeoutPeriod);
    asio::connect(*commandSocket,
                  resolver.resolve("localhost", std::to_string(5000)));
    asio::connect(*responseSocket,
                  resolver.resolve("localhost", std::to_string(5001)));

    ensureServerIsConnected(server);
    server.setTimeoutPeriod(failingTimeoutPeriod);
    std::this_thread::sleep_for(failingTimeoutPeriod + failingBufferPeriod);

    ASSERT_EQ(logger.count("Handshake timeout (" +
                           std::to_string(failingTimeoutPeriod.count()) +
                           " ms), disconnecting client"),
              2);
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions was successful
  {
    server::TcpServerMock server(5000, 5001, sufficientTimeoutPeriod);
    server.startServer();

    server::TcpClient client;
    bool isConnected = client.connect();
    ASSERT_TRUE(isConnected);

    // Give some time to the message to arrive
    logger.giveTimeToUpdate();
    ASSERT_TRUE(logger.contains("Handshake from client is valid"));
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Server can be shut if connexions was successful then dropped
  {
    server::TcpServerMock server(5000, 5001, sufficientTimeoutPeriod);
    server.startServer();

    server::TcpClient client;
    client.connect();
    ensureServerIsConnected(server);
    bool isDisconnected = client.disconnect();
    ASSERT_TRUE(isDisconnected);

    // Give some time to the message to arrive
    ensureServerIsDisconnected(server);
    logger.giveTimeToUpdate();
    ASSERT_TRUE(logger.contains("Disconnecting client"));
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // We can reconnect after disconnecting
  {
    server::TcpServerMock server(5000, 5001, sufficientTimeoutPeriod);
    server.startServer();

    server::TcpClient client;
    client.connect();
    ensureServerIsConnected(server);
    client.disconnect();
    ensureServerIsDisconnected(server);

    // Give some time
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    bool isConnected = client.connect();
    ASSERT_TRUE(isConnected);
    ensureServerIsConnected(server);

    logger.giveTimeToUpdate();
    ASSERT_EQ(logger.count("Handshake from client is valid"), 2);
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();
}
#endif // SKIP_LONG_TESTS

TEST(Server, AddDevices) {
  auto logger = TestLogger();

  // Happy path
  {
    server::TcpServerMock server(5000, 5001, sufficientTimeoutPeriod);
    server.startServer();

    server::TcpClient client;
    client.connect();

    // Add the devices
    bool isDelsysAdded = client.addDelsysDevice();
    bool isMagstimAdded = client.addMagstimDevice();
    ASSERT_TRUE(isDelsysAdded);
    ASSERT_TRUE(isMagstimAdded);

    // Give some time to the message to arrive
    ASSERT_TRUE(logger.contains("The device DelsysEmgDevice is now connected"));
    ASSERT_TRUE(
        logger.contains("The device MagstimRapidDevice is now connected"));
    ASSERT_TRUE(logger.contains(
        "The data collector DelsysEmgDataCollector is now streaming data"));
    logger.clear();
  }
  ASSERT_TRUE(logger.contains(
      "The data collector DelsysEmgDataCollector has stopped streaming data"));
  ASSERT_TRUE(
      logger.contains("The device DelsysEmgDevice is now disconnected"));
  ASSERT_TRUE(
      logger.contains("The device MagstimRapidDevice is now disconnected"));
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Add the same devices twice
  {
    server::TcpServerMock server(5000, 5001, sufficientTimeoutPeriod);
    server.startServer();

    server::TcpClient client;
    client.connect();

    // Add the devices
    bool isDelsysAdded = client.addDelsysDevice();
    bool isMagstimAdded = client.addMagstimDevice();
    ASSERT_TRUE(isDelsysAdded);
    ASSERT_TRUE(isMagstimAdded);

    // Add the devices again
    isDelsysAdded = client.addDelsysDevice();
    isMagstimAdded = client.addMagstimDevice();
    ASSERT_FALSE(isDelsysAdded);
    ASSERT_FALSE(isMagstimAdded);

    // Give some time to the message to arrive
    logger.giveTimeToUpdate();
    ASSERT_TRUE(logger.contains(
        "Cannot add the DelsysEmgDevice devise as it is already connected"));
    ASSERT_TRUE(logger.contains(
        "Cannot add the MagstimRapidDevice devise as it is already connected"));
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Remove devices by hand
  {
    server::TcpServerMock server(5000, 5001, sufficientTimeoutPeriod);
    server.startServer();

    server::TcpClient client;
    client.connect();

    // Add the devices
    bool isDelsysAdded = client.addDelsysDevice();
    bool isMagstimAdded = client.addMagstimDevice();
    ASSERT_TRUE(isDelsysAdded);
    ASSERT_TRUE(isMagstimAdded);
    logger.clear();

    // Remove the devices
    bool isDelsysRemoved = client.removeDelsysDevice();
    bool isMagstimRemoved = client.removeMagstimDevice();
    ASSERT_TRUE(isDelsysRemoved);
    ASSERT_TRUE(isMagstimRemoved);

    // Give some time to the message to arrive
    logger.giveTimeToUpdate();
    ASSERT_TRUE(
        logger.contains("The device DelsysEmgDevice is now disconnected"));
    ASSERT_TRUE(
        logger.contains("The device MagstimRapidDevice is now disconnected"));
  }
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();
}

TEST(Server, Recording) {
  auto logger = TestLogger();

  // Happy path
  {
    server::TcpServerMock server(5000, 5001, sufficientTimeoutPeriod);
    server.startServer();

    server::TcpClient client;
    client.connect();

    // Add the devices
    client.addDelsysDevice();
    client.addMagstimDevice();

    // Start recording
    bool isRecordingStarted = client.startRecording();
    ASSERT_TRUE(isRecordingStarted);

    // Give some time to the message to arrive
    logger.giveTimeToUpdate();
    ASSERT_TRUE(logger.contains(
        "The data collector DelsysEmgDataCollector is now recording"));
    logger.clear();
  }
  ASSERT_TRUE(logger.contains(
      "The data collector DelsysEmgDataCollector has stopped recording"));
  ASSERT_TRUE(logger.contains("Server has shut down"));
  logger.clear();

  // Start/Stop recording twice
  {
    server::TcpServerMock server(5000, 5001, sufficientTimeoutPeriod);
    server.startServer();

    server::TcpClient client;
    client.connect();

    // Add the devices
    bool isDelsysAdded = client.addDelsysDevice();
    bool isMagstimAdded = client.addMagstimDevice();
    ASSERT_TRUE(isDelsysAdded);
    ASSERT_TRUE(isMagstimAdded);

    // Start recording
    bool isRecordingStarted = client.startRecording();
    ASSERT_TRUE(isRecordingStarted);

    // Start recording again
    isRecordingStarted = client.startRecording();
    ASSERT_TRUE(isRecordingStarted);

    // Give some time to the message to arrive
    logger.giveTimeToUpdate();
    ASSERT_TRUE(logger.contains(
        "The data collector DelsysEmgDataCollector is already recording"));
    logger.clear();

    // Stop recording
    bool isRecordingStopped = client.stopRecording();
    ASSERT_TRUE(isRecordingStopped);

    // Stop recording again
    isRecordingStopped = client.stopRecording();
    ASSERT_TRUE(isRecordingStopped);

    // Give some time to the message to arrive
    logger.giveTimeToUpdate();
    ASSERT_TRUE(logger.contains(
        "The data collector DelsysEmgDataCollector is not recording"));
  }
  logger.clear();
}

TEST(Server, TrialData) {
  // TODO: Implement
}