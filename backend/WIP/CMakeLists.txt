cmake_minimum_required(VERSION 3.10)
project(TrignoDAQ)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the executable
set(SOURCE_FILES 
    main_emg_delsys_timings.cpp  
    main_emg_delsys.cpp 
    main_rehastim.cpp
)
foreach(SOURCE_FILE ${SOURCE_FILES})
    get_filename_component(EXECUTABLE_NAME ${SOURCE_FILE} NAME_WE)
    message(STATUS "Adding executable: ${EXECUTABLE_NAME}")
    add_executable(${EXECUTABLE_NAME} ${SOURCE_FILE})
    target_include_directories(${EXECUTABLE_NAME} PRIVATE
        ${Asio_INCLUDE_DIR}
    )
    # target_link_libraries(${EXECUTABLE_NAME} PRIVATE
    #     ws2_32
    # )
endforeach()