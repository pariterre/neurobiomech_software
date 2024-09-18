set(MODULE_UTILS ${STIMWALKER_NAME}_utils)
set(MODULE_UTILS ${MODULE_UTILS} PARENT_SCOPE)
project(${MODULE_UTILS})

# Add the relevant files
set(SRC_LIST_MODULE
    ${CMAKE_CURRENT_SOURCE_DIR}/Path.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/String.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Timestamp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UsbDevice.cpp
)

# Create the library
add_library(${MODULE_UTILS} STATIC ${SRC_LIST_MODULE})
set_target_properties(${MODULE_UTILS} PROPERTIES DEBUG_POSTFIX "_debug")

# Add the include
target_include_directories(${MODULE_UTILS} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
