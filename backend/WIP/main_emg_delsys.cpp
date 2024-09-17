#include <iostream>
#include <vector>
#include <cstring>
#include <string>
#include <stdexcept>
#include <array>
#include <fstream>
#include <filesystem>
#include <asio.hpp> // You'll need to install the ASIO library
#include <thread>
#include <chrono>

class BaseTrignoDaq
{
public:
    static const int BYTES_PER_CHANNEL = 4;
    static const std::string CMD_TERM;

    BaseTrignoDaq(const std::string &host, int cmd_port, int data_port, int total_channels, double timeout)
        : host_(host), cmd_port_(cmd_port), data_port_(data_port), total_channels_(total_channels), timeout_(timeout)
    {
        min_recv_size_ = total_channels_ * BYTES_PER_CHANNEL;
        initialize();
    }

    void start()
    {
        send_cmd("START");
    }

    void verifyFsrPosition()
    {
        send_cmd("SENSOR 14 TYPE");
        send_cmd("SENSOR 15 TYPE");
    }

    std::vector<std::vector<float>> read(int num_samples)
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

        for (int i = 0; i < num_samples; ++i)
        {
            for (int j = 0; j < total_channels_; ++j)
            {
                recording_file_ << reshaped_data[j][i] << ",";
            }
            recording_file_ << "\n";
        }
        recording_file_.flush();

        return reshaped_data;
    }

    void start_recording(const std::string &filename)
    {
        bool exist = std::filesystem::exists(filename);
        recording_file_.open(filename, std::ios::out | std::ios::trunc);
        if (!recording_file_.is_open())
        {
            throw std::runtime_error("Unable to open log file: " + filename);
        }

        for (int i = 0; i < total_channels_; ++i)
        {
            recording_file_ << "Channel_" << i << ",";
        }
        recording_file_ << "\n";
    }

    void stop_recording()
    {
        recording_file_.close();
    }

    void stop()
    {
        send_cmd("STOP");
    }

    void reset()
    {
        initialize();
    }

    ~BaseTrignoDaq()
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

protected:
    void initialize()
    {
        asio::ip::tcp::endpoint comm_endpoint(asio::ip::address::from_string("127.0.0.1"), cmd_port_);
        asio::ip::tcp::endpoint data_endpoint(asio::ip::address::from_string("127.0.0.1"), data_port_);

        comm_socket_.connect(comm_endpoint);
        std::array<char, 1024> buffer;
        comm_socket_.receive(asio::buffer(buffer));

        data_socket_.connect(data_endpoint);
    }

    void send_cmd(const std::string &command)
    {
        comm_socket_.send(asio::buffer(cmd(command)));
        std::array<char, 128> resp;
        comm_socket_.receive(asio::buffer(resp));
        validate(resp);
    }

    std::vector<char> cmd(const std::string &command)
    {
        std::string full_cmd = command + CMD_TERM;
        return std::vector<char>(full_cmd.begin(), full_cmd.end());
    }

    void validate(const std::array<char, 128> &response)
    {
        std::string s(response.data());
        if (s.find("OK") == std::string::npos)
        {
            std::cerr << "warning: TrignoDaq command failed: " << s << std::endl;
        }
    }

private:
    std::string host_;
    int cmd_port_;
    int data_port_;
    int total_channels_;
    double timeout_;
    int min_recv_size_;
    asio::io_context io_context_;
    asio::ip::tcp::socket comm_socket_{io_context_};
    asio::ip::tcp::socket data_socket_{io_context_};
    std::ofstream recording_file_;
};

const std::string BaseTrignoDaq::CMD_TERM = "\r\n\r\n";

class TrignoEMG : public BaseTrignoDaq
{
public:
    TrignoEMG(const std::pair<int, int> &channel_range, int samples_per_read, const std::string &units = "V",
              const std::string &host = "localhost", int cmd_port = 50040, int data_port = 50043, double timeout = 10)
        : BaseTrignoDaq(host, cmd_port, data_port, 16, timeout), channel_range_(channel_range), samples_per_read_(samples_per_read), scaler_(1.0)
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

    void set_channel_range(const std::pair<int, int> &channel_range)
    {
        channel_range_ = channel_range;
        num_channels_ = channel_range_.second - channel_range_.first + 1;
    }

    std::vector<std::vector<float>> read()
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

private:
    std::pair<int, int> channel_range_;
    int samples_per_read_;
    int num_channels_;
    double scaler_;
};

class TrignoAUX : public BaseTrignoDaq
{
public:
    TrignoAUX(const std::pair<int, int> &channel_range, int samples_per_read, const std::string &units = "V",
              const std::string &host = "localhost", int cmd_port = 50040, int data_port = 50044, double timeout = 10)
        : BaseTrignoDaq(host, cmd_port, data_port, 144, timeout), channel_range_(channel_range), samples_per_read_(samples_per_read), scaler_(1.0)
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

    void set_channel_range(const std::pair<int, int> &channel_range)
    {
        channel_range_ = channel_range;
        num_channels_ = channel_range_.second - channel_range_.first + 1;
    }

    std::vector<std::vector<float>> read()
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

private:
    std::pair<int, int> channel_range_;
    int samples_per_read_;
    int num_channels_;
    double scaler_;
};


int main()
{
    try
    {
        int runnumber;
        std::cout << "Enter how much seconds you want the trial to last : " << std::endl;
        std::cin >> runnumber;
        std::cout << "Starting " << runnumber << " seconds trial ..." << std::endl;
        TrignoEMG emg({0, 13}, 2000, "mV");
        TrignoAUX aux({0, 143}, 1/(0.0135/2), "mv");
        emg.start_recording("emg_data_sample.csv");
        aux.start_recording("aux_data_sample.csv");
        emg.start(); //only fire START command once all Trigno instances have been summonned and recording have been launched 
                     //(and with the master seesion only, otherwise it will fail)

        std::cout << std::endl;
        std::cout << std::endl;
        auto starttotalfirstrun = std::chrono::high_resolution_clock::now();
        std::thread t1firstrun([&emg](){
            auto startt1firstrun = std::chrono::high_resolution_clock::now();
            auto datafirstrun = emg.read();
            auto endt1firstrun = std::chrono::high_resolution_clock::now();
            auto durationt1firstrun = std::chrono::duration_cast<std::chrono::microseconds>(endt1firstrun - startt1firstrun);
            std::cout << "EMG data time for 2000hz : " << durationt1firstrun.count() << std::endl;
            std::cout << "Diff on 1s : " << durationt1firstrun.count() - 1000000 << " microseconds" << std::endl;
        });
        std::thread t2firstrun([&aux](){
            auto startt2firstrun = std::chrono::high_resolution_clock::now();
            auto dataauxfirstrun = aux.read();
            auto endt2firstrun = std::chrono::high_resolution_clock::now();
            auto durationt2firstrun = std::chrono::duration_cast<std::chrono::microseconds>(endt2firstrun - startt2firstrun);
            std::cout << "AUX data time for 148.148hz : " << durationt2firstrun.count() << std::endl;
            std::cout << "Diff on 1s : " << durationt2firstrun.count() - 1000000 << " microseconds" << std::endl;
        });
        t1firstrun.join();
        t2firstrun.join();
        std::cout << std::endl;
        auto endtotalfirstrun = std::chrono::high_resolution_clock::now();
        auto durationtotalfirstrun = std::chrono::duration_cast<std::chrono::microseconds>(endtotalfirstrun - starttotalfirstrun);
        std::cout << "Total data time for 1s, first run : " << durationtotalfirstrun.count() << std::endl;
        std::cout << "Diff on 1s, first run : " << durationtotalfirstrun.count() - 1000000 << " microseconds" << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;


        auto starttotal = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < runnumber; i++)
        {
            std::cout << "Run number " << i + 1 << std::endl;
            std::thread t1([&emg](){
                auto startt1 = std::chrono::high_resolution_clock::now();
                auto data = emg.read();
                auto endt1 = std::chrono::high_resolution_clock::now();
                auto durationt1 = std::chrono::duration_cast<std::chrono::microseconds>(endt1 - startt1);
                std::cout << "EMG data time for 2000hz : " << durationt1.count() << std::endl;
                std::cout << "Diff on 1s : " << durationt1.count() - 1000000 << " microseconds" << std::endl;
            });
            std::thread t2([&aux](){
                auto startt2 = std::chrono::high_resolution_clock::now();
                auto dataaux = aux.read();
                auto endt2 = std::chrono::high_resolution_clock::now();
                auto durationt2 = std::chrono::duration_cast<std::chrono::microseconds>(endt2 - startt2);
                std::cout << "AUX data time for 148.148hz : " << durationt2.count() << std::endl;
                std::cout << "Diff on 1s : " << durationt2.count() - 1000000 << " microseconds" << std::endl;
            });
            t1.join();
            t2.join();
            std::cout << std::endl;
        }
        auto endtotal = std::chrono::high_resolution_clock::now();
        auto durationtotal = std::chrono::duration_cast<std::chrono::microseconds>(endtotal - starttotal);
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << "Total data time for " << runnumber << "s : " << durationtotal.count() << std::endl;
        std::cout << "Diff on " << runnumber << "s : " << durationtotal.count() - (1000000*runnumber) << " microseconds" << std::endl;
        emg.stop();
        emg.stop_recording();
        aux.stop_recording();
        // return 0;
        // TrignoEMG emg({0, 13}, 2000, "mV");
        // emg.start_recording("emg_data_sample.csv");
        // emg.start();
        // for (size_t i = 0; i < 10; i++)
        // {
        //     auto data = emg.read();
        // }
        // emg.stop();
        // emg.stop_recording();
        // return 0;
        // for (const auto &channel : data)
        // {
        //     for (const auto &sample : channel)
        //     {
        //         std::cout << sample << " ";
        //     }
        //     std::cout << std::endl;
        // }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}