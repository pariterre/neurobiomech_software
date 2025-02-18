set(MODULE_ANALYZER ${NEUROBIO_NAME}_analyzer)
set(MODULE_ANALYZER ${MODULE_ANALYZER} PARENT_SCOPE)
project(${MODULE_ANALYZER})

# Add the relevant files
set(SRC_LIST_MODULE
    ${CMAKE_CURRENT_SOURCE_DIR}/Analyzers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/EventConditions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TimedEventsAnalyzer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CyclicTimedEventsAnalyzer.cpp
)

# Create the library
add_library(${MODULE_ANALYZER} STATIC ${SRC_LIST_MODULE})
set_target_properties(${MODULE_ANALYZER} PROPERTIES DEBUG_POSTFIX "_debug")

# Add the include
target_include_directories(${MODULE_ANALYZER} PRIVATE 
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link the library
target_link_libraries(${MODULE_ANALYZER} PRIVATE
    ${MODULE_UTILS}
    nlohmann_json::nlohmann_json
)
