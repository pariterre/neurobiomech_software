#include <algorithm>
#include <asio.hpp>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <map>
#include <stdexcept>
#include <thread>
#include <vector>

const uint8_t START_BYTE = 0xF0;
const uint8_t STOP_BYTE = 0x0F;
const uint8_t STUFFING_BYTE = 0x81;
const uint8_t STUFFING_KEY = 0x55;

// CRC-8 CCITT confirmations (a remplir)
const uint8_t CRC8_TABLE[256] = {
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, // 00..07
    0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D, // 08..15
    0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65, // 16..23
    0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D, // 24..31
    0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5, // 32..39
    0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD, // 40..47
    0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, // 48..55
    0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD, // 56..63
    0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2, // 64..71
    0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA, // 72..79
    0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2, // 80..87
    0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A, // 88..95
    0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, // 96..103
    0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A, // 104..111
    0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42, // 112..119
    0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A, // 120..127
    0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C, // 128..135
    0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4, // 136..143
    0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, // 144..151
    0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4, // 152..159
    0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, // 160..167
    0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44, // 168..175
    0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C, // 176..183
    0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34, // 184..191
    0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, // 192..199
    0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63, // 200..207
    0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, // 208..215
    0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13, // 216..223
    0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB, // 224..231
    0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83, // 232..239
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, // 240..247
    0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3  // 248..255
};

enum class Command : uint8_t {
  Init = 1,
  InitAck = 2,
  UnknownCommand = 3,
  Watchdog = 4,
  GetStimulationMode = 10,
  GetStimulationModeAck = 11,
  InitChannelListMode = 30,
  InitChannelListModeAck = 31,
  StartChannelListMode = 32,
  StartChannelListModeAck = 33,
  StopChannelListMode = 34,
  StopChannelListModeAck = 35,
  SinglePulse = 36,
  SinglePulseAck = 37,
  StimulationError = 38,
  InitPhaseTraining = 50,
  InitPhaseTrainingAck = 51,
  StartPhase = 52,
  StartPhaseAck = 53,
  PausePhase = 54,
  PausePhaseAck = 55,
  StopPhaseTraining = 56,
  StopPhaseTrainingAck = 57,
  PhaseResult = 58,
  ActualValues = 60,
  SetRotationDirection = 70,
  SetRotationDirectionAck = 71,
  SetSpeed = 72,
  SetSpeedAck = 73,
  SetGear = 74,
  SetGearAck = 75,
  SetKeyboardLock = 76,
  SetKeyboardLockAck = 77,
  StartBasicTraining = 80,
  StartBasicTrainingAck = 81,
  PauseBasicTraining = 82,
  PauseBasicTrainingAck = 83,
  ContinueBasicTraining = 84,
  ContinueBasicTrainingAck = 85,
  StopBasicTraining = 86,
  StopBasicTrainingAck = 87,
  MotomedCommandDone = 89,
  MotomedError = 90,
};

enum class ErrorCode : uint8_t {
  NoError = 0,
  TransferError = 1,
  ParameterError = 2,
  WrongModeError = 3,
  MotomedConnectionError = 4,
  IncompatibleProtocolVersion = 5,
  InvalidMotomedTrainer = 6,
  MotomedBusyError = 7,
  BusyError = 8,
};

class ScienceMode2Protocol {
public:
  ScienceMode2Protocol() : packet_number_(0) {}

  std::vector<uint8_t> createPacket(Command command,
                                    const std::vector<uint8_t> &data) {
    std::vector<uint8_t> packet;
    packet.push_back(START_BYTE);

    std::vector<uint8_t> payload = {packet_number_,
                                    static_cast<uint8_t>(command)};
    payload.insert(payload.end(), data.begin(), data.end());

    packet.push_back(STUFFING_BYTE);
    packet.push_back(calculateCRC8(payload) ^ 0x55);
    packet.push_back(STUFFING_BYTE);
    packet.push_back(static_cast<uint8_t>(payload.size() ^ 0x55));

    for (uint8_t byte : payload) {
      appendStuffed(packet, byte);
    }

    packet.push_back(STOP_BYTE);
    packet_number_++;

    return packet;
  }

  bool parsePacket(std::vector<uint8_t> &packet, Command &command,
                   std::vector<uint8_t> &data) {
    // std::cout << "packet before correction" << std::endl;
    // for (int i = 0; i < packet.size(); i++)
    // {
    //     std::cout << std::hex << static_cast<int>(packet[i]) << " ";
    // }
    // std::cout << std::endl;
    if (packet.back() != STOP_BYTE) {
      for (int i = 0; i < packet.size(); i++) {
        // std::cout << "correcting packet by " << i + 1 << std::endl;
        if (packet[i] == STOP_BYTE) {
          std::rotate(packet.begin(), packet.begin() + i + 1, packet.end());
          // for (int i = 0; i < packet.size(); i++)
          // {
          //     std::cout << std::hex << static_cast<int>(packet[i]) << " ";
          // }
          // std::cout << std::endl;
          break;
        }
        if (i == packet.size() - 1) {
          return false;
        }
      }
    }

    if (packet.front() != START_BYTE || packet.back() != STOP_BYTE) {
      return false;
    }

    std::vector<uint8_t> unstuffed = unstuffPacket(packet);

    std::cout << std::endl << "packet size : " << packet.size() << std::endl;
    for (int i = 0; i < packet.size(); i++) {
      std::cout << std::hex << static_cast<int>(packet[i]) << " ";
    }
    std::cout << std::endl
              << std::endl
              << "unstuffed size : " << unstuffed.size() << std::endl;
    for (int i = 0; i < unstuffed.size(); i++) {
      std::cout << std::hex << static_cast<int>(unstuffed[i]) << " ";
    }
    std::cout << std::endl << std::endl;

    if (unstuffed.size() < 6) { // min packet size
      return false;
    }

    uint8_t checksum = unstuffed[1];
    uint16_t length = static_cast<int>(unstuffed[2]);

    std::cout << "Payload lenght : " << length << std::endl;
    if (unstuffed.size() !=
        length + 4) { // 4 = checksum + data length + start + stop
      return false;
    }

    std::vector<uint8_t> payload(unstuffed.begin() + 3, unstuffed.end() - 1);
    if (calculateCRC8(payload) != checksum) {
      return false;
    }

    command = static_cast<Command>(payload[1]);
    data = std::vector<uint8_t>(payload.begin() + 2, payload.end());
    std::cout << "Packet succesfully parsed." << std::endl << std::endl;
    return true;
  }

  // for every command, remember create to a function that creates the packet
  // according to the protocol (pages 15-28)

  std::vector<uint8_t> createInitPacket(uint8_t version) {
    return createPacket(Command::InitAck, {version});
  }

  std::vector<uint8_t> createInitChannelListModePacket(
      uint8_t low_freq_factor, uint8_t active_channels,
      uint8_t active_low_freq_channels, uint16_t inter_pulse_interval,
      uint16_t main_stim_interval, uint8_t channel_execution) {
    std::vector<uint8_t> data = {
        low_freq_factor,
        active_channels,
        active_low_freq_channels,
        static_cast<uint8_t>((inter_pulse_interval / 0.5) - 3),
        static_cast<uint8_t>(main_stim_interval & 0xFF),
        static_cast<uint8_t>((main_stim_interval >> 8) & 0xFF),
        channel_execution};
    return createPacket(Command::InitChannelListMode, data);
  }

  std::vector<uint8_t> createActualValuesPacket(uint16_t angle, int16_t speed,
                                                int16_t torque) {
    std::vector<uint8_t> data = {
        static_cast<uint8_t>((angle >> 8) & 0xFF),  // Angle MSB
        static_cast<uint8_t>(angle & 0xFF),         // Angle LSB
        static_cast<uint8_t>((speed >> 8) & 0xFF),  // Speed MSB
        static_cast<uint8_t>(speed & 0xFF),         // Speed LSB
        static_cast<uint8_t>((torque >> 8) & 0xFF), // Torque MSB
        static_cast<uint8_t>(torque & 0xFF),        // Torque LSB
        0x0F                                        // Stop byte
    };
    return createPacket(Command::ActualValues, data);
  }

  std::vector<uint8_t> createStartChannelListModePacket(
      const std::vector<std::tuple<uint8_t, uint16_t, uint8_t>>
          &channel_params) {
    std::vector<uint8_t> data;
    for (const auto &params : channel_params) {
      data.push_back(std::get<0>(params));               // Mode
      data.push_back(std::get<1>(params) & 0xFF);        // Pulse width LSB
      data.push_back((std::get<1>(params) >> 8) & 0xFF); // Pulse width MSB
      data.push_back(std::get<2>(params));               // Current
    }
    return createPacket(Command::StartChannelListMode, data);
  }

private:
  uint8_t packet_number_;

  uint8_t calculateCRC8(const std::vector<uint8_t> &data) {
    uint8_t crc = 0;
    for (uint8_t byte : data) {
      crc = CRC8_TABLE[crc ^ byte];
    }
    return crc;
  }
  unsigned char CalcCRC8(const void *data, unsigned int length) {
    unsigned char crc = 0x00;

    const unsigned char *buf = (const unsigned char *)data;

    while (length--) {
      crc = CRC8_TABLE[crc ^ *buf++];
    }

    crc ^= 0x00;
    return crc;
  }

  void appendStuffed(std::vector<uint8_t> &packet, uint8_t byte) {
    if (byte == START_BYTE || byte == STOP_BYTE || byte == STUFFING_BYTE) {
      packet.push_back(STUFFING_BYTE);
      packet.push_back(byte ^ STUFFING_KEY);
    } else {
      packet.push_back(byte);
    }
  }

  std::vector<uint8_t> unstuffPacket(const std::vector<uint8_t> &packet) {
    std::cout << std::endl << "Unstuffing packet : " << std::endl;
    std::vector<uint8_t> unstuffed;
    bool escaped = false;

    for (int i = 0; i < packet.size(); i++) {
      if (escaped) {
        std::cout << "unstuffing byte" << std::endl;
        unstuffed.push_back(packet[i] ^ STUFFING_KEY);
        escaped = false;
      } else if (packet[i] == STUFFING_BYTE) {
        std::cout << "stuffing byte found" << std::endl;
        escaped = true;
      } else {
        std::cout << "normal byte" << std::endl;
        unstuffed.push_back(packet[i]);
      }
    }

    return unstuffed;
  }
};

class SerialCommunication {
public:
  SerialCommunication(const std::string &port, unsigned int baud_rate)
      : io_(), serial_(io_, port) {
    serial_.set_option(asio::serial_port_base::baud_rate(baud_rate));
    serial_.set_option(asio::serial_port_base::character_size(8));
    serial_.set_option(asio::serial_port_base::stop_bits(
        asio::serial_port_base::stop_bits::one));
    serial_.set_option(
        asio::serial_port_base::parity(asio::serial_port_base::parity::even));
    serial_.set_option(asio::serial_port_base::flow_control(
        asio::serial_port_base::flow_control::none));
  }

  void send(const std::vector<uint8_t> &data) {
    asio::write(serial_, asio::buffer(data));
  }

  std::vector<uint8_t> receive(int bytes_to_read,
                               std::chrono::milliseconds timeout) {
    std::vector<uint8_t> received_data(bytes_to_read);
    int bytes_read = 0;

    asio::async_read(serial_, asio::buffer(received_data),
                     [&bytes_read, &received_data](const auto &error,
                                                   int bytes_transferred) {
                       if (!error) {
                         bytes_read = bytes_transferred;
                       }
                     });

    if (io_.run_for(timeout)) {
      // auto tests = std::vector<uint8_t>(received_data.begin(),
      // received_data.begin() + bytes_read); std::cout << "Data : "; for (int i
      // = 0; i < bytes_read; i++) {
      //     std::cout << std::hex << static_cast<int>(tests[i]) << " ";
      // }
      // std::cout << std::dec << std::endl;
      return std::vector<uint8_t>(received_data.begin(),
                                  received_data.begin() + bytes_read);
    } else {
      throw std::runtime_error("Timeout while receiving data");
    }
  }

private:
  asio::io_service io_;
  asio::serial_port serial_;

  bool run_for(asio::io_context &io_context,
               std::chrono::milliseconds timeout) {
    io_.restart(); // Reset the io_context to ensure it's not in a stopped state
    auto timer = std::make_shared<asio::steady_timer>(io_);
    timer->expires_after(timeout);

    // Set the asynchronous wait operation
    timer->async_wait([timer](const asio::error_code &) {
      timer
          ->cancel(); // Ensure the timer is canceled once the operation is done
    });

    io_.run(); // Run the io_context to start processing asynchronous events

    // Return whether the timer has expired or not
    return timer->expires_after(std::chrono::milliseconds::zero());
  }
};

class RehaStim2Device {
public:
  RehaStim2Device(const std::string &port, unsigned int baud_rate)
      : protocol_(), serial_(port, baud_rate), connected_(false),
        stimulation_mode_(StimulationMode::StartMode) {}

  bool connect() {
    try {
      auto response = wait_for_response(500, 9);
      auto init_packet =
          protocol_.createInitPacket(1); // protocol version 1 maybe idk
      send_packet(init_packet);
      Command command;
      std::vector<uint8_t> data;
      std::cout << "parsing packet" << std::endl;
      if (protocol_.parsePacket(response, command, data)) {
        std::cout << std::endl << "checking command" << std::endl;
        std::cout << "command received : " << static_cast<int>(command)
                  << std::endl;
        std::cout << "command to receive : " << static_cast<int>(Command::Init)
                  << std::endl;
        std::cout << "version number of the protocol : "
                  << static_cast<int>(data[0]) << std::endl;
        if (command == Command::Init && data[0] == 1) {
          std::cout << "connection validated" << std::endl;
          connected_ = true;
          return true;
        }
      }
    } catch (const std::exception &e) {
      std::cerr << "Connection error: " << e.what() << std::endl;
    }
    return false;
  }

  void disconnect() { connected_ = false; }

  bool initChannelListMode(uint8_t low_freq_factor, uint8_t active_channels,
                           uint8_t active_low_freq_channels,
                           uint16_t inter_pulse_interval,
                           uint16_t main_stim_interval,
                           uint8_t channel_execution) {
    if (!connected_) {
      throw std::runtime_error("Device not connected");
    }

    auto packet = protocol_.createInitChannelListModePacket(
        low_freq_factor, active_channels, active_low_freq_channels,
        inter_pulse_interval, main_stim_interval, channel_execution);
    send_packet(packet);
    auto response = wait_for_response(100, 9);

    Command command;
    std::vector<uint8_t> data;
    if (protocol_.parsePacket(response, command, data)) {
      if (command == Command::InitChannelListModeAck && data[0] == 0) {
        stimulation_mode_ = StimulationMode::Initialized;
        return true;
      }
    }
    return false;
  }

  bool
  startChannelListMode(const std::vector<std::tuple<uint8_t, uint16_t, uint8_t>>
                           &channel_params) {
    if (!connected_ || stimulation_mode_ != StimulationMode::Initialized) {
      throw std::runtime_error("Device not connected or not initialized");
    }

    auto packet = protocol_.createStartChannelListModePacket(channel_params);
    send_packet(packet);
    auto response = wait_for_response(100, 9);

    Command command;
    std::vector<uint8_t> data;
    if (protocol_.parsePacket(response, command, data)) {
      if (command == Command::StartChannelListModeAck && data[0] == 0) {
        stimulation_mode_ = StimulationMode::Started;
        return true;
      }
    }
    return false;
  }

private:
  ScienceMode2Protocol protocol_;
  SerialCommunication serial_;
  bool connected_;
  enum class StimulationMode {
    StartMode,
    Initialized,
    Started
  } stimulation_mode_;

  void send_packet(const std::vector<uint8_t> &packet) { serial_.send(packet); }

  std::vector<uint8_t> wait_for_response(int timeout_ms, int num_bytes = 69) {
    return serial_.receive(num_bytes, std::chrono::milliseconds(timeout_ms));
  }
};

int main() {
  try {
    RehaStim2Device device("/dev/ttyUSB0", 460800);

    if (device.connect()) {
      std::cout << "Connected to RehaStim2 device" << std::endl;

      if (device.initChannelListMode(0, 0xFF, 0, 10000, 50000, 0)) {
        std::cout << "Channel list mode initialized" << std::endl;

        std::vector<std::tuple<uint8_t, uint16_t, uint8_t>> channel_params = {
            {0, 200, 20}, // Channel 1: Single pulse, 200 microsec, 20 mA
            {1, 300, 25}, // Channel 2: Doublet, 300 microsec, 25 mA
        };
        if (device.startChannelListMode(channel_params)) {
          std::cout << "Channel list mode started" << std::endl;

          std::this_thread::sleep_for(std::chrono::seconds(5));
        } else {
          std::cout << "Failed to start channel list mode" << std::endl;
        }
      } else {
        std::cout << "Failed to initialize channel list mode" << std::endl;
      }

      device.disconnect();
      std::cout << "Disconnected from RehaStim2 device" << std::endl;
    } else {
      std::cout << "Failed to connect to RehaStim2 device" << std::endl;
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  return 0;
}