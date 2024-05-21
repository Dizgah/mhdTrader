#ifndef WEBSOCKETMANAGER_H
#define WEBSOCKETMANAGER_H

#include "connection.h"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include<chrono>
#include<thread>
#include <vector>
#include <string>
#include <queue>
#include <list>
#include <memory>

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

#include "definition.h"
#include "connection.h"

#define URI                             "wss://ws.binaryws.com/websockets/v3?app_id=1089"
#define DEFAULT_WAIT_TIME               60
#define GET_TICK_STREAM                 "{\"ticks\":\""
#define GET_ACTIVE_SYMBOLS              "{ \"active_symbols\": \"brief\", \"product_type\": \"basic\" }"

typedef struct TICK_DATA
{
    double      ask;
    double      bid;
    double      quote;
    uint32_t    epoch;// 4 Byte is large enough for the purpose of this assesment!
//    uint8_t     pip_size;//this could be calculated from amount parameters as needed and has been removed to save the memory!
}tickData;

typedef websocketpp::lib::lock_guard<websocketpp::lib::mutex>scoped_lock;
typedef websocketpp::client<websocketpp::config::asio_tls_client>CLIENT;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context>context_ptr;

class WebsocketManager : public Connection
{
public:


    WebsocketManager();

    void run( const std::string &uri, size_t delay);

    void on_open( websocketpp::connection_hdl);
    // The close handler will signal that we should stop
    void on_close( websocketpp::connection_hdl);
    void on_fail(websocketpp::connection_hdl);
    // The TLS handler will initial secure connection parameters
    context_ptr on_tls_init( websocketpp::connection_hdl);
    // The Message handler will be fired whenever a data packet has been recieved
    void on_message( websocketpp::connection_hdl, CLIENT::message_ptr msg);




    void stablishStream();
    bool initTickStream( std::string tickSymbol);
    void lazyLogger( size_t delay);
    void takeANap( size_t seconds);
    bool isConnected( size_t waitTime = DEFAULT_WAIT_TIME);
    bool activeSymbolsFed();

    virtual ~WebsocketManager();

private:

    CLIENT                      client;
    websocketpp::connection_hdl conHandler;
    websocketpp::lib::mutex     lock;
    bool                        connected;
    bool                        symbolsRecieved;
    std::vector<std::string>    symbols;
    std::queue< TICK_DATA>      ticks;
};

#endif // WEBSOCKETMANAGER_H
