cmake_minimum_required(VERSION 3.20)
project(project1)

set(CMAKE_CXX_STANDARD 23)

set(MyTarget ${PROJECT_NAME})
set(MyTargetServerBin ${MyTarget}_serverbin)
set(MyTargetClientBin ${MyTarget}_clientbin)
set(MyTargetServerTxt ${MyTarget}_servertxt)
set(MyTargetClientTxt ${MyTarget}_clienttxt)

add_executable(${MyTargetServerBin})
target_sources(${MyTargetServerBin} PRIVATE serverbin.cpp)
target_link_libraries(${MyTargetServerBin} PRIVATE ws2_32)
add_executable(${MyTargetServerTxt})
target_sources(${MyTargetServerTxt} PRIVATE servertxt.cpp)
target_link_libraries(${MyTargetServerTxt} PRIVATE ws2_32)

add_executable(${MyTargetClientBin})
target_sources(${MyTargetClientBin} PRIVATE clientbin.cpp)
target_link_libraries(${MyTargetClientBin} PRIVATE ws2_32)
add_executable(${MyTargetClientTxt})
target_sources(${MyTargetClientTxt} PRIVATE clienttxt.cpp)
target_link_libraries(${MyTargetClientTxt} PRIVATE ws2_32)