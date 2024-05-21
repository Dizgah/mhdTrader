#include "websocketManager.h"

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

WebsocketManager::WebsocketManager()
{
    // set up access channels to only log interesting things
    client.clear_access_channels( websocketpp::log::alevel::all);
    client.set_access_channels( websocketpp::log::alevel::connect);
    client.set_access_channels( websocketpp::log::alevel::disconnect);
    client.set_access_channels( websocketpp::log::alevel::app);

    // Initialize the Asio transport policy
    client.init_asio();

    // Bind the handlers we are using
    client.set_open_handler( websocketpp::lib::bind( &WebsocketManager::on_open,this,websocketpp::lib::placeholders::_1));
    client.set_close_handler( websocketpp::lib::bind( &WebsocketManager::on_close,this,websocketpp::lib::placeholders::_1));
    client.set_fail_handler( websocketpp::lib::bind( &WebsocketManager::on_fail,this,websocketpp::lib::placeholders::_1));
    client.set_tls_init_handler( websocketpp::lib::bind( &WebsocketManager::on_tls_init, this, websocketpp::lib::placeholders::_1));
    client.set_message_handler( websocketpp::lib::bind( &WebsocketManager::on_message,this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
}


void WebsocketManager::run( const std::string &uri, size_t delay)
{
    websocketpp::lib::error_code    error;
    CLIENT::connection_ptr          con = client.get_connection( uri, error);


    if( error)
    {
        client.get_alog().write(websocketpp::log::alevel::app, "Get Connection Error: "+error.message());

        return;
    }

    // Grab a handle for this connection so we can talk to it in a thread
    conHandler = con->get_handle();

    // Queue the connection. No DNS queries or network connections will be
    // made until the io_service event loop is run.
    client.connect( con);

    // Create a thread to run the ASIO io_service event loop
    websocketpp::lib::thread asio_thread( &CLIENT::run, &client);

    // Create a thread to transcieve initial information with websocket server
    websocketpp::lib::thread stablishStream_thread( &WebsocketManager::stablishStream, this);

    // Create a thread to log the received data after passing a specified dealy
    websocketpp::lib::thread lazyLogger_thread( &WebsocketManager::lazyLogger, this, delay);

    asio_thread.detach();
    stablishStream_thread.detach();
    lazyLogger_thread.detach();
}

// The open handler will signal that we are ready to start
void WebsocketManager::on_open( websocketpp::connection_hdl)
{
    client.get_alog().write( websocketpp::log::alevel::app, "Connection opened!");

    scoped_lock guard( lock);
    connected = true;
}

// The close handler will signal that we should stop
void WebsocketManager::on_close( websocketpp::connection_hdl)
{
    client.get_alog().write( websocketpp::log::alevel::app, "Connection closed!");

    scoped_lock guard(lock);
    connected = false;
}

// The fail handler will signal that we should stop
void WebsocketManager::on_fail(websocketpp::connection_hdl)
{
    client.get_alog().write(websocketpp::log::alevel::app, "Connection failed, stopping !");

    scoped_lock guard(lock);
    connected = false;
}

// The TLS handler will initial secure connection parameters
context_ptr WebsocketManager::on_tls_init( websocketpp::connection_hdl)
{
    context_ptr ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);

    try
    {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::single_dh_use);

        ctx->set_verify_mode(boost::asio::ssl::verify_none);
    }
    catch( std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    return ctx;
}

// The Message handler will be fired whenever a data packet has been recieved
void WebsocketManager::on_message( websocketpp::connection_hdl, CLIENT::message_ptr msg)
{
//    rapidjson::Document document;


//    document.Parse( msg->get_payload().c_str());

//    if( document.HasParseError())
//    {
//        std::cout << "Error  : " << document.GetParseError()  << '\n' << "Offset : " << document.GetErrorOffset() << '\n';

//        return ;
//    }
//    else if( document.HasMember("active_symbols"))
//    {
//        rapidjson::Value &activeSymbols = document["active_symbols"];


//        for( auto &symbol : activeSymbols.GetArray())
//        {
//            if( !symbol.HasMember("symbol"))
//            {
//                std::cout << "Error  : " << document.GetParseError()  << '\n' << "Offset : " << document.GetErrorOffset() << '\n';

//                return ;
//            }

//            rapidjson::Value::ConstMemberIterator itr = symbol.FindMember("symbol");


//            if( itr->value.IsString())
//            {
//                symbols.push_back( itr->value.GetString());
//            }
//        }

//        scoped_lock guard( lock);
//        symbolsRecieved = true;
//    }
//    else if( document.HasMember("msg_type"))
//    {
//        rapidjson::Value::ConstMemberIterator itr = document.FindMember("msg_type");


//        if(( itr->value.IsString()) && ( !strncmp( itr->value.GetString(), "tick", 4)) && ( document.HasMember("tick")))
//        {
//            rapidjson::Value& tick = document["tick"];
//            TICK_DATA tmpData;


//            tmpData.ask = tick.FindMember("ask")->value.GetDouble();
//            tmpData.bid = tick.FindMember("bid")->value.GetDouble();
//            tmpData.quote = tick.FindMember("quote")->value.GetDouble();
//            tmpData.epoch = tick.FindMember("epoch")->value.GetUint();

//            ticks.push( tmpData);
//        }
//    }
}

void WebsocketManager::stablishStream()
{
    websocketpp::lib::error_code ec;


    if( !isConnected())
    {
        client.get_alog().write(websocketpp::log::alevel::app, "Connection couldn't be stablished!");
    }

    std::string msg = GET_ACTIVE_SYMBOLS;

    client.send( conHandler, msg,websocketpp::frame::opcode::text, ec);

    if( ec)
    {
        client.get_alog().write(websocketpp::log::alevel::app, "Send Error: "+ec.message());

        return;
    }

    if( !isConnected())
    {
        client.get_alog().write(websocketpp::log::alevel::app, "Connection couldn't be stablished!");
    }
}


bool WebsocketManager::initTickStream( std::string tickSymbol)
{
    std::string msg = GET_TICK_STREAM + tickSymbol + "\"}";
    websocketpp::lib::error_code ec;


    for( auto &itr : symbols)
    {
        if( itr.compare( tickSymbol) == 0)
        {
            client.send(conHandler, msg,websocketpp::frame::opcode::text,ec);

            if( ec)
            {
                client.get_alog().write(websocketpp::log::alevel::app, "Send Error: " + ec.message());

                return false;
            }
            else
            {
                return true;
            }
        }
    }

    //TODO:: should return not found or not active error code
    return false;
}

void WebsocketManager::lazyLogger( size_t delay)
{
    while( true)
    {
        if( ticks.empty())
        {
            takeANap( 1);
        }
        else
        {
            uint32_t    now         = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            TICK_DATA   firstTick   = ticks.front();


            if( now >= ( firstTick.epoch + delay))
            {
                std::cout << "now: " << now << std::endl;
                std::cout << "Tick_epoch: " << firstTick.epoch << std::endl;
                std::cout << "Tick_ask: " << firstTick.ask << std::endl;
                std::cout << "Tick_bid: " << firstTick.bid << std::endl;
                std::cout << "Tick_quote: " << firstTick.quote << std::endl;

                ticks.pop();
            }
        }
    }
}

void WebsocketManager::takeANap( size_t seconds)
{
    std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::seconds( seconds));
}

bool WebsocketManager::isConnected( size_t waitTime)
{
    //TODO: limit waiting time with switching to the timedlock or so...
    do
    {
        scoped_lock guard(lock);
        if( connected)
        {
            break;
        }

    }while( true);

    //TODO: add if waiting time is expired or it is connected!
    return true;
}

bool WebsocketManager::activeSymbolsFed()
{
    scoped_lock guard(lock);
    if( symbolsRecieved)
    {
        return true;
    }
    else
    {
        return false;
    }
}




WebsocketManager::~WebsocketManager()
{

}
