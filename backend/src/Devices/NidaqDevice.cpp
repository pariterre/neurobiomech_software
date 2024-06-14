#include "Devices/NidaqDevice.h"

#include "Devices/Generic/Exceptions.h"
#include "Devices/Generic/CollectorData.h"

#include <iostream>
namespace STIMWALKER_NAMESPACE{ namespace devices {

NidaqDevice::NidaqDevice(
    int nbChannels,
    int frameRate
) : m_nbChannels(nbChannels),
    m_frameRate(frameRate),
    m_isConnected(false),
    m_isRecording(false){
}

NidaqDevice::~NidaqDevice() {
    dispose();
}

int NidaqDevice::getNbChannels() const {
    return m_nbChannels;
}

int NidaqDevice::getFrameRate() const {
    return m_frameRate;
}

bool NidaqDevice::getIsConnected() const {
    return m_isConnected;
}

void NidaqDevice::connect() {
    if (getIsConnected()) {
        throw DeviceIsConnectedException("The device is already connected");
    }

    connectInternal();

    m_isConnected = true;
}

void NidaqDevice::connectInternal() {
    // TODO: Implement this method
}

void NidaqDevice::disconnect() {
    if (isRecording()) {
        throw DeviceIsRecordingException("The device is currently recording");
    }

    if (!getIsConnected()) {
        throw DeviceIsNotConnectedException("The device is not connected");
    }

    disconnectInternal();

    m_isConnected = false;
}

void NidaqDevice::disconnectInternal() {
    // TODO: Implement this method
}

void NidaqDevice::dispose() {
    try {
        stopRecording();
    } catch (const DeviceIsNotRecordingException& e) {
        // Do nothing
    }

    try {
        disconnect();
    } catch (const DeviceIsNotConnectedException& e) {
        // Do nothing
    }
}

bool NidaqDevice::isRecording() const {
    return m_isRecording;
}

void NidaqDevice::startRecording() {
    if (!getIsConnected()) {
        throw DeviceIsNotConnectedException("The device is not connected");
    }
    if (isRecording()) {
        throw DeviceIsRecordingException("The device is already recording");
    }

    startRecordingInternal();
    
    std::lock_guard<std::mutex> lock(m_recordingMutex);
    m_isRecording = true;
}

void NidaqDevice::startRecordingInternal() {
    // TODO: Implement this method
}

void NidaqDevice::stopRecording() {
    if (!isRecording()) {
        throw DeviceIsNotRecordingException("The device is not recording");
    }

    stopRecordingInternal();

    std::lock_guard<std::mutex> lock(m_recordingMutex);
    m_isRecording = false;
}

void NidaqDevice::stopRecordingInternal() {
    // TODO: Implement this method
}

int NidaqDevice::onNewData(std::function<void(const CollectorData& newData)> onDataCollected) {
    static int listenerId = 0;
        
    std::lock_guard<std::mutex> lock(m_recordingMutex);
    m_listeners[listenerId] = std::function<void(const CollectorData&)>(onDataCollected);

    return listenerId++;
}

void NidaqDevice::removeListener(int listenerId) {
    std::lock_guard<std::mutex> lock(m_recordingMutex);
    m_listeners.erase(listenerId);
}

std::vector<CollectorData> NidaqDevice::getData() const {
    return m_data;
}

CollectorData NidaqDevice::getData(int index) const {
    return m_data[index];
}

void NidaqDevice::notifyListeners(const CollectorData& newData) {
    std::lock_guard<std::mutex> lock(m_recordingMutex);
    for (const auto& listener : m_listeners) {
        listener.second(newData);
    }
}

// Mock implementation
NidaqDeviceMock::NidaqDeviceMock(
    int nbChannels,
    int frameRate
) : NidaqDevice(nbChannels, frameRate){
}

void NidaqDeviceMock::startRecordingInternal() {

    {
        std::lock_guard<std::mutex> lock(m_recordingMutex);
        m_generateData = true;
    }

    m_recordingThread = std::thread(&NidaqDeviceMock::generateData, this);
}

void NidaqDeviceMock::stopRecordingInternal() {
    {
        std::lock_guard<std::mutex> lock(m_recordingMutex);
        m_isRecording = false;
        m_generateData = false;
    }
    if (m_recordingThread.joinable()) m_recordingThread.join();
}

void NidaqDeviceMock::generateData() {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(m_recordingMutex);
            if (!m_generateData) break;
        }
        
        // Generate data
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        auto data = std::vector<double>(getNbChannels(), 0.0);
        CollectorData newData(now, data);
        notifyListeners(newData);

        std::this_thread::sleep_for(std::chrono::milliseconds(1 / m_frameRate));
    }
}


}}