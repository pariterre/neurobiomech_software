project(${STIMWALKER_NAME}_tests)

# Download gtest if necessary
set(RELATIVE_PATH_TO_GTEST external/googletest)
find_path(GTEST_INCLUDE_DIR "gtest.h" HINTS
    ${CMAKE_CURRENT_SOURCE_DIR}/../${RELATIVE_PATH_TO_GTEST}/googletest/include/gtest
)

if((NOT GTEST_INCLUDE_DIR) OR (NOT EXISTS ${GTEST_INCLUDE_DIR}))
    find_package(Git QUIET)
    if (NOT GIT_FOUND)
        message(FATAL_ERROR "Unable to find gtest and it could not be download since Git was not found!")
    endif()
    
    # we couldn't find the header files for gtest or they don't exist
    message("Unable to find gtest, I'll try to automatically download it")

    # we have a submodule setup for gtest, assume it is under external/gtest
    # now we need to clone this submodule
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init -- ${STIMWALKER_ROOT_FOLDER}/${RELATIVE_PATH_TO_GTEST}
                    WORKING_DIRECTORY ${STIMWALKER_ROOT_FOLDER}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "${GIT_EXECUTABLE} submodule update --init -- ${STIMWALKER_ROOT_FOLDER}/external/gtest failed, please checkout submodules manually")
    endif()
    
    find_path(GTEST_INCLUDE_DIR "gtest.h" HINTS
        ${CMAKE_CURRENT_SOURCE_DIR}/../${RELATIVE_PATH_TO_GTEST}/googletest/include/gtest
    )
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
set(BUILD_GMOCK OFF)
set(INSTALL_GTEST OFF)
add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/../${RELATIVE_PATH_TO_GTEST}
    ${CMAKE_BINARY_DIR}/gtest
    EXCLUDE_FROM_ALL
)

##############
# Unit Tests
##############
enable_testing()

set(TEST_SRC_FILES
    ${CMAKE_SOURCE_DIR}/test/test_data.cpp
    ${CMAKE_SOURCE_DIR}/test/test_lokomat.cpp
    ${CMAKE_SOURCE_DIR}/test/test_magstim.cpp
    ${CMAKE_SOURCE_DIR}/test/test_nidaq.cpp
    ${CMAKE_SOURCE_DIR}/test/test_utils.cpp
)
add_executable(${PROJECT_NAME} ${TEST_SRC_FILES})

# headers for the project
target_include_directories(${PROJECT_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Standard linking to gtest stuff
add_dependencies(${PROJECT_NAME} 
    ${MODULE_UTILS}
    ${MODULE_DEVICES}
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${MODULE_UTILS}
    ${MODULE_DEVICES}
    nlohmann_json::nlohmann_json
    "gtest_main"
)
if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Setupapi
    )
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
    include(CodeCoverage)
    setup_target_for_coverage(${STIMWALKER_NAME}_coverage
        "${PROJECT_NAME}" "coverage")

    SET(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
    SET(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
endif() #CMAKE_BUILD_TYPE STREQUAL "Coverage"

set(ALL_TESTS ${PROJECT_NAME})

# This is so you can do 'make test' to see all your tests run, instead of
# manually running the executable runUnitTests to see those specific tests.
add_test(UnitTests ${ALL_TESTS})
