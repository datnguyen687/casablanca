cmake_minimum_required(VERSION 3.6)
project(application)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

FIND_PACKAGE(Boost COMPONENTS system thread timer chrono REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/Login)

set(SOURCE_FILES main.cpp Login/CLoginPage.cc)
add_executable(application ${SOURCE_FILES})
target_link_libraries(application jsoncpp wt wthttp cpprest ${Boost_LIBRARIES})