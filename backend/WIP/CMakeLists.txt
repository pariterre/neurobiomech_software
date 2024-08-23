cmake_minimum_required(VERSION 3.10)
project(TrignoDAQ)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(trigno_daq_timings main_emg_delsys_timings_test.cpp)
add_executable(trigno_daq_normal main_emg_delsys.cpp)
add_executable(rehastim main_rehastim.cpp)

target_include_directories(trigno_daq_timings PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/$("asio2\\asio\\include")
)
target_link_libraries(trigno_daq_timings PUBLIC 
    ws2_32
)
target_include_directories(trigno_daq_normal PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/$("asio2\\asio\\include")
)
target_link_libraries(trigno_daq_normal PUBLIC 
    ws2_32
)


set(BOOST_INCLUDEDIR "C:\\local\\boost_1_86_0")
set(BOOST_LIBRARYDIR "C:\\local\\boost_1_86_0\\lib64-msvc-14.1")
set(BOOST_ROOT "C:\\local\\boost_1_86_0\\boost")

target_include_directories(rehastim PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/$("C:\\boost_1_86_0\\boost_1_86_0")
)
target_link_libraries(rehastim PUBLIC 
    ws2_32
)