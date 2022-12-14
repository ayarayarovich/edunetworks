cmake_minimum_required(VERSION 3.20)
project(project2)

set(CMAKE_CXX_STANDARD 23)

set(MyTarget ${PROJECT_NAME}_A)
set(MyTargetServer ${MyTarget}_server)
set(MyTargetClient ${MyTarget}_client)

add_executable(${MyTargetServer})
target_sources(${MyTargetServer} PRIVATE server.cpp)
target_link_libraries(${MyTargetServer} PRIVATE ws2_32)

add_executable(${MyTargetClient})
target_sources(${MyTargetClient} PRIVATE client.cpp)
target_link_libraries(${MyTargetClient} PRIVATE ws2_32)
