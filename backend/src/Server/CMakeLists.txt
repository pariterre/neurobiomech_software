set(MODULE_SERVER ${NEUROBIO_NAME}_server)
set(MODULE_SERVER ${MODULE_SERVER} PARENT_SCOPE)
project(${MODULE_SERVER})

# Add the relevant files
set(SRC_LIST_MODULE
${CMAKE_CURRENT_SOURCE_DIR}/TcpClient.cpp
${CMAKE_CURRENT_SOURCE_DIR}/TcpServer.cpp
)

# Create the library
add_library(${MODULE_SERVER} STATIC ${SRC_LIST_MODULE})
set_target_properties(${MODULE_SERVER} PROPERTIES DEBUG_POSTFIX "_debug")

# Add the include
target_include_directories(${MODULE_SERVER} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${Asio_INCLUDE_DIR}
)

# Link the library
target_link_libraries(${MODULE_SERVER} PRIVATE
    ${MODULE_DATA}
    ${MODULE_UTILS}
    ${MODULE_DEVICES}
    nlohmann_json::nlohmann_json
)
