project(${STIMWALKER_NAME}_devices)
set(DEVICES_MODULE_NAME ${STIMWALKER_NAME}_devices PARENT_SCOPE)

# Add the relevant files
set(SRC_LIST_MODULE
    "${CMAKE_CURRENT_SOURCE_DIR}/NidaqDevice.cpp"
)

# Create the library
add_library(${PROJECT_NAME} STATIC "${SRC_LIST_MODULE}")
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "_debug")

# Add the include
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
    "${${STIMWALKER_NAME}_BINARY_DIR}/include"
    "${EIGEN3_INCLUDE_DIR}"
)
