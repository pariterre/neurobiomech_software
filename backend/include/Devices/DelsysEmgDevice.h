#ifndef DELSYSEMGDEVICE_H
#define DELSYSEMGDEVICE_H

#include <string>
#include <vector>
#include <array>
#include <asio.hpp>
#include <fstream>

#include "Devices/Generic/Device.h"
#include "Devices/Generic/Collector.h"

class BaseTrignoDaq
{
public:
    static const int BYTES_PER_CHANNEL = 4;
    static const std::string CMD_TERM;

    BaseTrignoDaq(const std::string &host, int cmd_port, int data_port, int total_channels, double timeout);
    virtual ~BaseTrignoDaq();

    void start();
    std::vector<std::vector<float>> read(int num_samples);
    void stop();
    void reset();
    void start_recording(const std::string &filename);
    void stop_recording();

protected:
    void initialize();
    void send_cmd(const std::string &command);
    std::vector<char> cmd(const std::string &command);
    void validate(const std::array<char, 128> &response);

private:
    std::string host_;
    int cmd_port_;
    int data_port_;
    int total_channels_;
    double timeout_;
    int min_recv_size_;
    asio::io_context io_context_;
    asio::ip::tcp::socket comm_socket_;
    asio::ip::tcp::socket data_socket_;
    std::ofstream recording_file_;
    bool is_recording_;
};

class TrignoEMG : public BaseTrignoDaq
{
public:
    TrignoEMG(const std::pair<int, int> &channel_range, int samples_per_read, const std::string &units = "V",
              const std::string &host = "localhost", int cmd_port = 50040, int data_port = 50043, double timeout = 10);

    void set_channel_range(const std::pair<int, int> &channel_range);
    std::vector<std::vector<float>> read();

private:
    std::pair<int, int> channel_range_;
    int samples_per_read_;
    int num_channels_;
    double scaler_;
};

namespace STIMWALKER_NAMESPACE::devices
{
    class DelsysEmgDevice : public Device, public Collector
    {
    public:
        DelsysEmgDevice(const std::pair<int, int> &channelRange, int frameRate, const std::string &units = "V",
                        const std::string &host = "localhost", int cmdPort = 50040, int dataPort = 50043, double timeout = 10);

        ~DelsysEmgDevice();

        int getNbChannels() const override;

        int getFrameRate() const override;

        bool getIsConnected() const override;

        void connect() override;

        virtual void connectInternal();

        void disconnect() override;

        virtual void disconnectInternal();

        void dispose() override;

        bool isRecording() const override;

        void startRecording() override;

        virtual void startRecordingInternal();

        void stopRecording() override;

        virtual void stopRecordingInternal();

        int onNewData(std::function<void(const CollectorData &newData)> callback) override;

        void removeListener(int listenerId) override;

        std::vector<CollectorData> getData() const override;

        CollectorData getData(int index) const override;

    protected:
        void notifyListeners(const CollectorData &newData);

        bool m_isConnected;
        bool m_isRecording = false;

        int m_nbChannels;
        int m_frameRate;

        std::vector<CollectorData> m_data_;
        std::vector<std::function<void(const CollectorData &newData)>> m_listeners;

        std::thread m_recordingThread;
        std::mutex m_recordingMutex;

        std::unique_ptr<TrignoEMG> emg_;
    };
}
#endif // DELSYSEMGDEVICE_H