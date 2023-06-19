cmake_minimum_required( VERSION 3.5)

project( mhdTrader LANGUAGES C CXX)

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

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})

include_directories( ${PROJECT_DIR}/)
include_directories( ${INC}/)
include_directories( ${LIB}/)
include_directories( ${WEBSOCKETPP}/)
include_directories( ${RAPIDJSON}/)
include_directories( ${GUMBO_PARSER}/)
include_directories( ${Boost_INCLUDE_DIR})
include_directories( ${OPENSSL_INCLUDE_DIR})

set( GUMBO_PARSER_SRC
    "lib/gumbo-parser/attribute.c"
    "lib/gumbo-parser/char_ref.c"
    "lib/gumbo-parser/error.c"
    "lib/gumbo-parser/parser.c"
    "lib/gumbo-parser/string_buffer.c"
    "lib/gumbo-parser/string_piece.c"
    "lib/gumbo-parser/tag.c"
    "lib/gumbo-parser/tokenizer.c"
    "lib/gumbo-parser/utf8.c"
    "lib/gumbo-parser/util.c"
    "lib/gumbo-parser/vector.c"
)

add_library( gumbo STATIC ${GUMBO_PARSER_SRC})
#link_directories( ${LIB}/ ${GUMBO_PARSER}/)

target_include_directories( gumbo PUBLIC ${GUMBO_PARSER})

add_executable( mhdTrader main.cpp)
target_link_libraries ( mhdTrader ${PROJECT_LINK_LIBS} Threads::Threads ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto ${CURL_LIBRARIES} gumbo)
