cmake_minimum_required(VERSION 3.14)

project(neurobio VERSION 0.1.0)
set(NEUROBIO_ROOT_FOLDER ${PROJECT_SOURCE_DIR})
set (CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

# Set all the compile options
option(BUILD_DOC 
    "Build documentation" OFF)
option(BUILD_TESTS 
    "Build all tests." OFF)
option(BUILD_BINARIES
    "Build all binary examples." OFF)

# Set a default build type to 'Release' if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' by default.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build.")

    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY
        STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo" "Coverage")
endif()

# Prepare linkings
set (NEUROBIO_NAME ${PROJECT_NAME})

# Add the configuration include
configure_file(${CMAKE_SOURCE_DIR}/include/neurobioConfig.h.in
    ${CMAKE_BINARY_DIR}/include/neurobioConfig.h)


find_package(nlohmann_json 3.2.0 CONFIG REQUIRED)
find_package(Asio REQUIRED)


# Compilation directories
set(${NEUROBIO_NAME}_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Add linker
add_subdirectory("src/Utils")
add_subdirectory("src/Data")
add_subdirectory("src/Devices")
add_subdirectory("src/Server")

# Doc
if (BUILD_DOC)
    add_subdirectory("doc")
endif (BUILD_DOC)

# Testing
if (BUILD_TESTS)
    add_subdirectory("test")
endif()

if (BUILD_BINARIES)
    add_subdirectory("run")
endif()