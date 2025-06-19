cmake_minimum_required(VERSION 3.31)
project(AlarmClock_mk3)

set(CMAKE_CXX_STANDARD 20)

add_definitions(
        -D SERIAL_BAUD_RATE=0
        -D CORE_DEBUG_LEVEL=5
        -D DEBUG
)

add_executable(mock
        mock.cpp
        src/main.cpp)
