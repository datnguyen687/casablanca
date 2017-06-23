cmake_minimum_required(VERSION 3.6)
project(server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(${CMAKE_SOURCE_DIR}/HTTPServer)
include_directories(${CMAKE_SOURCE_DIR}/UserDatabase)

FIND_PACKAGE(Boost COMPONENTS system thread timer chrono regex REQUIRED)
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
link_directories(${Boost_LIBRARY_DIR})

FIND_PACKAGE(libmongocxx REQUIRED)
include_directories(${LIBMONGOCXX_INCLUDE_DIR})
include_directories("/usr/local/include/mongocxx/v_noabi")

FIND_PACKAGE(libbsoncxx REQUIRED)
include_directories(${LIBBSONCXX_INCLUDE_DIR})
include_directories("/usr/local/include/bsoncxx/v_noabi")

set(SOURCE_FILES main.cpp HTTPServer/CHTTPServer.cc UserDatabase/CUserDatabase.cc)
add_executable(server ${SOURCE_FILES})
target_link_libraries(server cpprest jsoncpp ${Boost_LIBRARIES} ${LIBMONGOCXX_LIBRARIES} ${LIBBSONCXX_LIBRARIES})