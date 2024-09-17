set(MODULE_DEVICES "${STIMWALKER_NAME}_devices")
set(MODULE_DEVICES ${MODULE_DEVICES} PARENT_SCOPE)
project(${MODULE_DEVICES})

# Add the relevant files
set(SRC_LIST_MODULE
    "${CMAKE_CURRENT_SOURCE_DIR}/Data/DataCollection.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Data/CollectorData.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/NidaqDevice.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DelsysEmgDevice.cpp"
)

# Create the library
add_library(${MODULE_DEVICES} STATIC "${SRC_LIST_MODULE}")
set_target_properties(${MODULE_DEVICES} PROPERTIES DEBUG_POSTFIX "_debug")

# Add the include
target_include_directories(${MODULE_DEVICES} PRIVATE 
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link the library
target_link_libraries(${MODULE_DEVICES} PRIVATE
    ${MODULE_UTILS}
    nlohmann_json::nlohmann_json
)
