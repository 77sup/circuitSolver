cmake_minimum_required(VERSION 3.1.0)
project(SOIDIERFIRE)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
#绝对路径写法：include_directories(${CMAKE_SOURCE_DIR/include}),#CMAKE_SOURCE_DIR等价于根目录
include_directories(${CMAKE_SOURCE_DIR}/include)


add_executable(test_solver main.cpp src/iscas89_parser.cpp src/circuit_graph.cpp src/log.cpp src/solver.cpp
 src/circuitsolver.cpp)
