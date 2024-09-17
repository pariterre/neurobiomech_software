#include "Devices/DelsysEmgDevice.h"
#include "Devices/Generic/Exceptions.h"
#include "Devices/Data/CollectorData.h"
#include <iostream>
#include <cstring>
#include <stdexcept>
using namespace STIMWALKER_NAMESPACE::devices;

const std::string BaseTrignoDaq::CMD_TERM = "\r\n\r\n";

BaseTrignoDaq::BaseTrignoDaq(const std::string &host, int cmd_port, int data_port, int total_channels, double timeout)
    : host_(host), cmd_port_(cmd_port), data_port_(data_port), total_channels_(total_channels), timeout_(timeout),
      comm_socket_(io_context_), data_socket_(io_context_), is_recording_(false)
{
    min_recv_size_ = total_channels_ * BYTES_PER_CHANNEL;
    initialize();
}

BaseTrignoDaq::~BaseTrignoDaq()
{
    try
    {
        comm_socket_.close();
    }
    catch (...)
    {
        // do nothing
    }
}

void BaseTrignoDaq::start()
{
    send_cmd("START");
}

std::vector<std::vector<float>> BaseTrignoDaq::read(int num_samples)
{
    int l_des = num_samples * min_recv_size_;
    int l = 0;
    std::vector<char> packet(l_des, 0);

    while (l < l_des)
    {
        try
        {
            int received = data_socket_.receive(asio::buffer(packet.data() + l, l_des - l));
            l += received;
        }
        catch (std::exception &e)
        {
            l = packet.size();
            std::fill(packet.begin() + l, packet.end(), 0);
            throw std::runtime_error("Device disconnected.");
        }
    }

    std::vector<float> data(l_des / BYTES_PER_CHANNEL);
    std::memcpy(data.data(), packet.data(), l_des);

    std::vector<std::vector<float>> reshaped_data(total_channels_, std::vector<float>(num_samples));
    for (int i = 0; i < num_samples; ++i)
    {
        for (int j = 0; j < total_channels_; ++j)
        {
            reshaped_data[j][i] = data[i * total_channels_ + j];
        }
    }

    if (is_recording_)
    {
        for (int i = 0; i < num_samples; ++i)
        {
            for (int j = 0; j < total_channels_; ++j)
            {
                recording_file_ << reshaped_data[j][i] << ",";
            }
            recording_file_ << "\n";
        }
        recording_file_.flush();
    }

    return reshaped_data;
}

void BaseTrignoDaq::start_recording(const std::string &filename)
{
    if (!is_recording_)
    {
        recording_file_.open("DelsysEmgDevice-" + filename, std::ios::out | std::ios::trunc);
        if (!recording_file_.is_open())
        {
            throw std::runtime_error("Unable to open log file: DelsysEmgDevice-" + filename);
        }

        for (int i = 0; i < total_channels_; ++i)
        {
            recording_file_ << "Channel_" << i << ",";
        }
        recording_file_ << "\n";

        is_recording_ = true;
    }
}

void BaseTrignoDaq::stop_recording()
{
    if (is_recording_)
    {
        recording_file_.close();
        is_recording_ = false;
    }
}

void BaseTrignoDaq::stop()
{
    send_cmd("STOP");
}

void BaseTrignoDaq::reset()
{
    initialize();
}

void BaseTrignoDaq::initialize()
{
    asio::ip::tcp::endpoint comm_endpoint(asio::ip::address::from_string(host_), cmd_port_);
    asio::ip::tcp::endpoint data_endpoint(asio::ip::address::from_string(host_), data_port_);

    comm_socket_.connect(comm_endpoint);
    std::array<char, 1024> buffer;
    comm_socket_.receive(asio::buffer(buffer));

    data_socket_.connect(data_endpoint);
}

void BaseTrignoDaq::send_cmd(const std::string &command)
{
    comm_socket_.send(asio::buffer(cmd(command)));
    std::array<char, 128> resp;
    comm_socket_.receive(asio::buffer(resp));
    validate(resp);
}

std::vector<char> BaseTrignoDaq::cmd(const std::string &command)
{
    std::string full_cmd = command + CMD_TERM;
    return std::vector<char>(full_cmd.begin(), full_cmd.end());
}

void BaseTrignoDaq::validate(const std::array<char, 128> &response)
{
    std::string s(response.data());
    if (s.find("OK") == std::string::npos)
    {
        std::cerr << "warning: TrignoDaq command failed: " << s << std::endl;
    }
}

TrignoEMG::TrignoEMG(const std::pair<int, int> &channel_range, int samples_per_read, const std::string &units,
                     const std::string &host, int cmd_port, int data_port, double timeout)
    : BaseTrignoDaq(host, cmd_port, data_port, 14, timeout), channel_range_(channel_range), samples_per_read_(samples_per_read), scaler_(1.0)
{
    if (units == "mV")
    {
        scaler_ = 1000.0;
    }
    else if (units == "normalized")
    {
        scaler_ = 1 / 0.011;
    }
    set_channel_range(channel_range);
}

void TrignoEMG::set_channel_range(const std::pair<int, int> &channel_range)
{
    channel_range_ = channel_range;
    num_channels_ = channel_range_.second - channel_range_.first + 1;
}

std::vector<std::vector<float>> TrignoEMG::read()
{
    auto data = BaseTrignoDaq::read(samples_per_read_);
    std::vector<std::vector<float>> channel_data(num_channels_, std::vector<float>(samples_per_read_));

    for (int i = 0; i < num_channels_; ++i)
    {
        channel_data[i] = data[channel_range_.first + i];
    }

    for (auto &channel : channel_data)
    {
        for (auto &sample : channel)
        {
            sample *= scaler_;
        }
    }

    return channel_data;
}

DelsysEmgDevice::DelsysEmgDevice(const std::pair<int, int> &channelRange, int frameRate, const std::string &units,
                                 const std::string &host, int cmdPort, int dataPort, double timeout) : m_frameRate(frameRate), m_isConnected(false), m_isRecording(false)
{
    DelsysEmgDevice::connect();
}

DelsysEmgDevice::~DelsysEmgDevice()
{
    dispose();
}

void DelsysEmgDevice::dispose()
{
    try
    {
        stopRecording();
    }
    catch (const DeviceIsNotRecordingException &e)
    {
        // Do nothing
    }

    try
    {
        disconnect();
    }
    catch (const DeviceIsNotConnectedException &e)
    {
        // Do nothing
    }
}

int DelsysEmgDevice::getNbChannels() const
{
    return 14;
}

int DelsysEmgDevice::getFrameRate() const
{
    return m_frameRate;
}

bool DelsysEmgDevice::getIsConnected() const
{
    return m_isConnected;
}

void DelsysEmgDevice::connect()
{
    if (getIsConnected())
    {
        throw DeviceIsConnectedException("The device is already connected");
    }

    connectInternal();

    m_isConnected = true;
}

void DelsysEmgDevice::connectInternal()
{
    emg_ = std::make_unique<TrignoEMG>(std::make_pair(0, 15), 2000, "mV", "host");
}

void DelsysEmgDevice::disconnect()
{
    if (isRecording())
    {
        throw DeviceIsRecordingException("The device is currently recording");
    }

    if (!getIsConnected())
    {
        throw DeviceIsNotConnectedException("The device is not connected");
    }

    disconnectInternal();

    m_isConnected = false;
}

void DelsysEmgDevice::disconnectInternal()
{
}

bool DelsysEmgDevice::isRecording() const
{
    return m_isRecording;
}

void DelsysEmgDevice::startRecording()
{
    if (isRecording())
    {
        throw DeviceIsRecordingException("The device is already recording");
    }

    startRecordingInternal();

    m_isRecording = true;
}

void DelsysEmgDevice::startRecordingInternal()
{
    emg_->start_recording("emg_data.csv");
}

void DelsysEmgDevice::stopRecording()
{
    if (!isRecording())
    {
        throw DeviceIsNotRecordingException("The device is not recording");
    }

    stopRecordingInternal();

    m_isRecording = false;
}

void DelsysEmgDevice::stopRecordingInternal()
{
    emg_->stop_recording();
}

int DelsysEmgDevice::onNewData(std::function<void(const CollectorData &newData)> callback)
{
    m_listeners.push_back(callback);
    return m_listeners.size() - 1;
}

void DelsysEmgDevice::removeListener(int listenerId)
{
    m_listeners.erase(m_listeners.begin() + listenerId);
}

std::vector<CollectorData> DelsysEmgDevice::getData() const
{
    return m_data_;
}

CollectorData DelsysEmgDevice::getData(int index) const
{
    return m_data_[index];
}

void DelsysEmgDevice::notifyListeners(const CollectorData &newData)
{
    for (auto &listener : m_listeners)
    {
        listener(newData);
    }
}