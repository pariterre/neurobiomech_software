project(${STIMWALKER_NAME}_utils)
set(UTILS_MODULE_NAME ${STIMWALKER_NAME}_utils PARENT_SCOPE)

# Add the relevant files
set(SRC_LIST_MODULE
    "${CMAKE_CURRENT_SOURCE_DIR}/Benchmark.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Error.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Path.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/String.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Timer.cpp"
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
