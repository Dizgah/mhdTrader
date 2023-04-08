cmake_minimum_required(VERSION 3.5)

project(mhdTrader LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set (PROJECT_DIR ./)

find_package(OpenSSL REQUIRED)
find_package (Threads)

set(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREAD ON)
find_package(Boost REQUIRED system thread)

include_directories (${PROJECT_DIR}/inc ${PROJECT_DIR}/lib ${PROJECT_DIR}/lib/websocketpp ${PROJECT_DIR}/lib/rapidjson ${Boost_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR})
link_directories (${PROJECT_DIR}/lib)

#compile all *.cpp source files under src folder
file (GLOB SOURCES "./*.cpp" "./src/*.cpp")

add_executable(mhdTrader main.cpp)
target_link_libraries (mhdTrader ${PROJECT_LINK_LIBS} Threads::Threads ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto)
