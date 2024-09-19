#ifndef __STIMWALKER_DEVICES_USB_DEVICE_H__
#define __STIMWALKER_DEVICES_USB_DEVICE_H__

#include <iostream>

#include <asio.hpp>
#include <vector>

#include "Utils/CppMacros.h"

// https://github.com/nicolasmcnair/magpy/blob/master/magpy/magstim.py#L129
// https://github.com/nigelrogasch/MAGIC/blob/master/magstim.m#L301

enum Commands
{
    CHANGE_POKE_INTERVAL,
    PRINT
};

/// @brief A class representing a USB device
/// @details This class provides a way to list all USB devices connected to the system and get their information
/// @note This class is only available on Windows and Linux
class UsbDevice
{
    /// Constructors
public:
    /// @brief Constructor
    /// @param port The port name of the device
    /// @param vid The vendor ID of the device
    /// @param pid The product ID of the device
    UsbDevice(const std::string &port, const std::string &vid, const std::string &pid);

    /// @brief Copy constructor
    /// @param other The other UsbDevice object to copy
    UsbDevice(const UsbDevice &other);

    /// @brief Factory method to create a UsbDevice object from a vendor ID and product ID. Throws an exception if the device is not found
    /// @param vid The vendor ID of the device
    /// @param pid The product ID of the device
    /// @return A UsbDevice object with the specified vendor ID and product ID. Throws an exception if the device is not found
    static UsbDevice fromVidAndPid(const std::string &vid, const std::string &pid);

public:
    /// Members with Get accessors

    /// @brief Get the port name of the device
    /// @return The port name of the device
    DECLARE_PROTECTED_MEMBER(std::string, Port)

    /// @brief Get the vendor ID of the device
    /// @return The vendor ID of the device
    DECLARE_PROTECTED_MEMBER(std::string, Vid)

    /// @brief Get the product ID of the device
    /// @return The product ID of the device
    DECLARE_PROTECTED_MEMBER(std::string, Pid)

    /// Private members

    /// @brief Get the serial port of the device
    /// @return The serial port of the device
    DECLARE_PRIVATE_MEMBER(std::unique_ptr<asio::serial_port>, SerialPort)

    /// @brief Get the async context of the device
    /// @return The async context of the device
    DECLARE_PRIVATE_MEMBER(std::unique_ptr<asio::io_context>, SerialPortContext)

    /// @brief Get the async context of the command loop
    /// @return The async context of the command loop
    DECLARE_PRIVATE_MEMBER(std::unique_ptr<asio::io_context>, Context)

    /// @brief Get the mutex
    /// @return The mutex
    DECLARE_PRIVATE_MEMBER(std::mutex, Mutex)

    /// @brief Get how long to wait before sending the PING command
    /// @return How long to wait before sending the PING command
    DECLARE_PRIVATE_MEMBER(std::chrono::milliseconds, PokeInterval)

    /// @brief Worker thread to keep the device alive
    DECLARE_PRIVATE_MEMBER(std::thread, Worker)

    /// @brief Get the keep-alive timer
    /// @return The keep-alive timer
    DECLARE_PRIVATE_MEMBER(std::unique_ptr<asio::steady_timer>, KeepAliveTimer)

    /// Methods
public:
    /// @brief Connect the device
    void connect();

    /// @brief Disconnect the device
    void disconnect();

    /// @brief Send a command to the device
    /// @param command The command to send to the device
    /// @param data The data to send to the device
    void send(Commands command, const std::any &data);
    void send(Commands command, const char *data)
    {
        send(command, std::string(data));
    }

protected:
    /// @brief Connect to the ubs device. This is expected to run on an async thread
    void _initialize();

    /// @brief Parse a command received from the user and send to the device
    /// @param command The command to parse
    /// @param data The data to parse
    void _parseCommand(Commands command, const std::any &data);

    /// @brief Set a worker thread to keep the device alive
    void _keepAlive(const std::chrono::milliseconds &timeout);

    /// @brief Change the interval at which the device is poked
    void _changePokeInterval(std::chrono::milliseconds interval);

    /// @brief Set the "RTS" mode of the communication. [isFast] to true is faster but less reliable.
    /// @param isFast True to enable fast mode, false to disable it
    /// @note This methods emulates the useRTS signal from Python
    void _setFastCommunication(bool isFast);

    /// Static helper methods
public:
    /// @brief Static method to list all USB devices connected to the system
    /// @return A vector of UsbDevice objects representing the connected USB devices
    static std::vector<UsbDevice> listAllUsbDevices();

    /// @brief Equality operator
    /// @param other The other UsbDevice object to compare with
    /// @return True if the two objects are equal, false otherwise
    bool operator==(const UsbDevice &other) const;
};

#endif // __STIMWALKER_DEVICES_USB_DEVICE_H__