cmake_minimum_required(VERSION 3.10)
project(TrignoDAQ)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the executable
set(SOURCE_FILES 
    example_delsys.cpp
    example_devices.cpp
    example_magstim.cpp
    example_server.cpp
    example_old_rehastim.cpp
    example_old_lokomat.cpp
    main_server.cpp
)
foreach(SOURCE_FILE ${SOURCE_FILES})
    get_filename_component(EXECUTABLE_NAME ${SOURCE_FILE} NAME_WE)
    message(STATUS "Adding executable: ${EXECUTABLE_NAME}")
    add_executable(${EXECUTABLE_NAME} ${SOURCE_FILE})
    
    target_include_directories(${EXECUTABLE_NAME} PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${Asio_INCLUDE_DIR}
    )
    add_dependencies(${EXECUTABLE_NAME} 
        ${MODULE_ANALYZER}
        ${MODULE_UTILS}
        ${MODULE_DATA}
        ${MODULE_DEVICES}
        ${MODULE_SERVER}
    )
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE
        ${MODULE_ANALYZER}
        ${MODULE_UTILS}
        ${MODULE_DATA}
        ${MODULE_DEVICES}
        ${MODULE_SERVER}
        nlohmann_json::nlohmann_json
    )
    if (WIN32)
        target_link_libraries(${EXECUTABLE_NAME} PRIVATE
            Setupapi
        )
    endif()
endforeach()