cmake_minimum_required(VERSION 3.5)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package (Threads)
find_package(Boost COMPONENTS system filesystem thread REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

add_executable(tcp-echo-client.bin tcpclient_sync.cpp)
add_executable(echo-server.bin echo-server.cpp)

target_link_libraries(tcp-echo-client.bin ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(echo-server.bin ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
