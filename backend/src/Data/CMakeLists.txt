set(MODULE_DATA ${STIMWALKER_NAME}_data)
set(MODULE_DATA ${MODULE_DATA} PARENT_SCOPE)
project(${MODULE_DATA})

# Add the relevant files
set(SRC_LIST_MODULE
    ${CMAKE_CURRENT_SOURCE_DIR}/DataPoint.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TimeSeries.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/FixedTimeSeries.cpp
)

# Create the library
add_library(${MODULE_DATA} STATIC ${SRC_LIST_MODULE})
set_target_properties(${MODULE_DATA} PROPERTIES DEBUG_POSTFIX "_debug")
add_dependencies(${MODULE_DATA} ${MODULE_UTILS})

# Add the include
target_include_directories(${MODULE_DATA} PRIVATE 
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${Asio_INCLUDE_DIR}
)

# Link the library
target_link_libraries(${MODULE_DATA} PRIVATE
    ${MODULE_UTILS}
    nlohmann_json::nlohmann_json
)
