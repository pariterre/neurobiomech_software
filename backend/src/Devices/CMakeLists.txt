set(MODULE_DEVICES ${STIMWALKER_NAME}_devices)
set(MODULE_DEVICES ${MODULE_DEVICES} PARENT_SCOPE)
project(${MODULE_DEVICES})

# Add the relevant files
set(SRC_LIST_MODULE
    ${CMAKE_CURRENT_SOURCE_DIR}/Data/DataDevices.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Data/DataPoint.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Data/TimeSeries.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Generic/Device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Generic/AsyncDevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Generic/DataCollector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Generic/AsyncDataCollector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Generic/TcpDevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Generic/SerialPortDevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Generic/UsbDevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/NidaqDevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DelsysEmgDevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MagstimRapidDevice.cpp
)

# Create the library
add_library(${MODULE_DEVICES} STATIC ${SRC_LIST_MODULE})
set_target_properties(${MODULE_DEVICES} PROPERTIES DEBUG_POSTFIX "_debug")
add_dependencies(${MODULE_DEVICES} ${MODULE_UTILS})

# Add the include
target_include_directories(${MODULE_DEVICES} PRIVATE 
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${Asio_INCLUDE_DIR}
)

# Link the library
target_link_libraries(${MODULE_DEVICES} PRIVATE
    ${MODULE_UTILS}
    nlohmann_json::nlohmann_json
)
