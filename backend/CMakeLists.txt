cmake_minimum_required(VERSION 3.8)

project(stimwalker VERSION 1.11.2)
set(STIMWALKER_ROOT_FOLDER ${PROJECT_SOURCE_DIR})
set (CMAKE_CXX_STANDARD 17)

# Set all the compile options
option(BUILD_DOC 
    "Build documentation" OFF)
option(BUILD_TESTS 
    "Build all tests." OFF)

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
find_package(Eigen3 REQUIRED)

# Manage options
option(SKIP_ASSERT
    "Some checks slow the code down at run-time, but provide more robust
    code. By default, it is ON faster but less secure" ON)

# Prepare add library
set(SRC_LIST "src/main.cpp")
add_executable(${STIMWALKER_NAME} ${SRC_LIST})
set_target_properties(${STIMWALKER_NAME} PROPERTIES DEBUG_POSTFIX "_debug")

# Compilation directories
set(${STIMWALKER_NAME}_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")

# Create the configuration header file associated with the asked options
configure_file(${CMAKE_SOURCE_DIR}/include/stimwalkerConfig.h.in
"${${STIMWALKER_NAME}_BINARY_DIR}/include/stimwalkerConfig.h"
)

# Add headers
# Include directories when building the library:
target_include_directories(${STIMWALKER_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
    "${${STIMWALKER_NAME}_BINARY_DIR}/include"
    "${EIGEN3_INCLUDE_DIR}"
)

# Installation directories
set(${STIMWALKER_NAME}_BIN_FOLDER "${CMAKE_INSTALL_PREFIX}/bin/${STIMWALKER}")
set_target_properties(${STIMWALKER_NAME} PROPERTIES
    INSTALL_RPATH "${${STIMWALKER_NAME}_BIN_FOLDER}"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

# Add the different modules of stimwalker
set(STIMWALKER_MODULE_NAMES)

add_subdirectory("src/Utils")
list(APPEND STIMWALKER_MODULE_NAMES "${UTILS_MODULE_NAME}")

add_subdirectory("src/Devices")
list(APPEND STIMWALKER_MODULE_NAMES "${DEVICES_MODULE_NAME}")

# Add linker
target_link_libraries(${STIMWALKER_NAME}
    "${STIMWALKER_MODULE_NAMES}"
)

# install target
install(
    TARGETS ${STIMWALKER_NAME} EXPORT stimwalkerTargets
    RUNTIME DESTINATION "${${STIMWALKER_NAME}_BIN_FOLDER}"
)

# Doc
if (BUILD_DOC)
    add_subdirectory("doc")
endif (BUILD_DOC)

# Testing
if (BUILD_TESTS)
    add_subdirectory("test")
endif()
