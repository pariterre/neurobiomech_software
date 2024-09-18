cmake_minimum_required(VERSION 3.14)

project(stimwalker VERSION 1.11.2)
set(STIMWALKER_ROOT_FOLDER ${PROJECT_SOURCE_DIR})
set (CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

# Set all the compile options
option(BUILD_DOC 
    "Build documentation" OFF)
option(BUILD_TESTS 
    "Build all tests." OFF)
option(BUILD_WIP
    "Build all work in progress." OFF)

# Set a default build type to 'Release' if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' by default.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build.")

    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY
        STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo" "Coverage")
endif()

# Prepare linkings
set (STIMWALKER_NAME ${PROJECT_NAME})
find_package(nlohmann_json 3.2.0 CONFIG REQUIRED)
find_package(Asio REQUIRED)

# Prepare add library
set(SRC_LIST "src/main.cpp")
add_executable(${STIMWALKER_NAME} ${SRC_LIST})
set_target_properties(${STIMWALKER_NAME} PROPERTIES DEBUG_POSTFIX "_debug")

# Compilation directories
set(${STIMWALKER_NAME}_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Add headers
# Include directories when building the library:
target_include_directories(${STIMWALKER_NAME} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${Asio_INCLUDE_DIR}
)

# Add linker
add_subdirectory("src/Utils")
add_subdirectory("src/Devices")
target_link_libraries(${STIMWALKER_NAME} PRIVATE
    ${MODULE_UTILS}
    ${MODULE_DEVICES}
    nlohmann_json::nlohmann_json
)

# install target
set_target_properties(${STIMWALKER_NAME} PROPERTIES
    INSTALL_RPATH ${CMAKE_INSTALL_BINDIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
)
install(TARGETS stimwalker
    EXPORT stimwalkerTargets          # Export the target so it can be found
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}  # Install the shared library
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}  # For static library (if needed)
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}  # For executables (if any)
)

# Doc
if (BUILD_DOC)
    add_subdirectory("doc")
endif (BUILD_DOC)

# Testing
if (BUILD_TESTS)
    add_subdirectory("test")
endif()

if (BUILD_WIP)
    add_subdirectory("WIP")
endif()