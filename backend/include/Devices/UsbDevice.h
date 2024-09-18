#include <iostream>

#include "Utils/CppMacros.h"
#include <vector>

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

    /// @brief Factory method to create a UsbDevice object from a vendor ID and product ID. Throws an exception if the device is not found
    /// @param vid The vendor ID of the device
    /// @param pid The product ID of the device
    /// @return A UsbDevice object with the specified vendor ID and product ID. Throws an exception if the device is not found
    static UsbDevice fromVidAndPid(const std::string &vid, const std::string &pid);

    /// Attributes and accessors
public:
    /// @brief Get the port name of the device
    /// @return The port name of the device
    DECLARE_PROTECTED_MEMBER(std::string, Port)

    /// @brief Get the vendor ID of the device
    /// @return The vendor ID of the device
    DECLARE_PROTECTED_MEMBER(std::string, Vid)

    /// @brief Get the product ID of the device
    /// @return The product ID of the device
    DECLARE_PROTECTED_MEMBER(std::string, Pid)

    /// Methods
public:
    /// @brief Connect to the device
    /// @return True if the connection was successful, false otherwise
    bool connect();

    /// Static methods
public:
    /// @brief Static method to list all USB devices connected to the system
    /// @return A vector of UsbDevice objects representing the connected USB devices
    static std::vector<UsbDevice> listAll();

    /// @brief Equality operator
    /// @param other The other UsbDevice object to compare with
    /// @return True if the two objects are equal, false otherwise
    bool operator==(const UsbDevice &other) const;
};