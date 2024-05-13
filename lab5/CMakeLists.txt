cmake_minimum_required(VERSION 3.5)   # CMake version check
project(server)                       # Create project 

set(CMAKE_CXX_STANDARD 14)            # Enable c++14 standard

# set(CMAKE_BUILD_TYPE "Debug")

file(GLOB SOURCE_FILES "src/*.cpp")
file(GLOB SOURCE_FILES2 "src2/*.cpp")

set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

find_package(Boost 1.74 REQUIRED COMPONENTS system filesystem thread)

# headers directory 
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/include2")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

add_executable(client ${SOURCE_FILES2})


if(Boost_FOUND)
    target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

    target_include_directories(client PUBLIC ${Boost_INCLUDE_DIR})
    target_link_libraries(client ${Boost_LIBRARIES})

    message(STATUS "Found BOOST ${Boost_VERSION_STRING}")
else()
    message(STATUS "WEBSOCKET BOOST Not Found")
endif()


find_package(SQLite3 REQUIRED)

if (SQLITE3_FOUND)
    target_link_libraries (${PROJECT_NAME} SQLite::SQLite3)
    message(STATUS "Found SQLite3")
else()
    message(STATUS "SQLite3 Not Found")
endif (SQLITE3_FOUND)
