cmake_minimum_required(VERSION 3.5)   # CMake version check
project(server)                       # Create project 

set(CMAKE_CXX_STANDARD 14)            # Enable c++14 standard

set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

find_package(Boost 1.74 REQUIRED COMPONENTS system filesystem thread)

# headers directory 
include_directories("${PROJECT_SOURCE_DIR}/include")

add_executable(${PROJECT_NAME} src/main_server.cpp src/dbEditor.cpp src/server.cpp)

add_executable(client src/main_client.cpp)

add_executable(test test/main_test.cpp)

if(Boost_FOUND)
    target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

    target_include_directories(client PUBLIC ${Boost_INCLUDE_DIR})
    target_link_libraries(client ${Boost_LIBRARIES})
    
    target_include_directories(test PUBLIC ${Boost_INCLUDE_DIR})
    target_link_libraries(test ${Boost_LIBRARIES})
else()
    message(STATUS "WEBSOCKET BOOST Not Found")
endif(Boost_FOUND)


find_package(SQLite3 REQUIRED)

if(SQLITE3_FOUND)
    target_link_libraries (${PROJECT_NAME} SQLite::SQLite3)
else()
    message(STATUS "SQLite3 Not Found")
endif(SQLITE3_FOUND)
