cmake_minimum_required( VERSION 3.5)

project( mhdTrader LANGUAGES CXX)

set( CMAKE_CXX_STANDARD 17)
set( CMAKE_CXX_STANDARD_REQUIRED ON)

set( PROJECT_DIR .)
set( INC ${PROJECT_DIR}/inc)
set( SRC ${PROJECT_DIR}/src)
set( LIB ${PROJECT_DIR}/lib)
set( WEBSOCKETPP ${LIB}/websocketpp)
set( RAPIDJSON ${LIB}/rapidjson)
set( GUMBO_PARSER ${LIB}/gumbo-parser)

find_package( OpenSSL REQUIRED)
find_package( Threads)

set( Boost_USE_STATIC_LIBS ON)
SET( Boost_USE_MULTITHREAD ON)
find_package( Boost REQUIRED system thread)

include_directories( ${PROJECT_DIR}/)
include_directories( ${INC}/)
include_directories( ${LIB}/)
include_directories( ${WEBSOCKETPP}/)
include_directories( ${RAPIDJSON}/)
include_directories( ${GUMBO_PARSER}/)
include_directories( ${Boost_INCLUDE_DIR})
include_directories( ${OPENSSL_INCLUDE_DIR})

set( GUMBO_PARSER_SRC
    "${GUMBO_PARSER}/attribute.c"
    "${GUMBO_PARSER}/char_ref.c"
    "${GUMBO_PARSER}/error.c"
    "${GUMBO_PARSER}/parser.c"
    "${GUMBO_PARSER}/string_buffer.c"
    "${GUMBO_PARSER}/string_piece.c"
    "${GUMBO_PARSER}/tag.c"
    "${GUMBO_PARSER}/tokenizer.c"
    "${GUMBO_PARSER}/utf8.c"
    "${GUMBO_PARSER}/util.c"
    "${GUMBO_PARSER}/vector.c"
)

add_library( gumbo STATIC ${GUMBO_PARSER_SRC})

link_directories( ${LIB}/ ${GUMBO_PARSER}/)

#compile all *.cpp source files under src folder
file ( GLOB SOURCES "./*.cpp" "./src/*.cpp")

add_executable( mhdTrader main.cpp)
target_link_libraries ( mhdTrader ${PROJECT_LINK_LIBS} Threads::Threads ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto ${GUMBO_PARSER_SRC})
