cmake_minimum_required(VERSION 3.6)
project(unit_test)
#enable_testing()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

FIND_PACKAGE(Boost COMPONENTS system thread timer chrono regex unit_test_framework  REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

set(SOURCE_FILES main.cpp)
add_executable(unit_test ${SOURCE_FILES})
target_compile_definitions(unit_test PRIVATE "BOOST_TEST_DYN_LINK=1")
target_link_libraries(unit_test cpprest jsoncpp ${Boost_LIBRARIES})

# declares a test with our executable
#add_test(NAME test1 COMMAND unit_test)