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

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/ostreamwrapper.h>

#include <curl/curl.h>
#include "gumbo.h"

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

class WsHandler
{

public:

    typedef websocketpp::lib::lock_guard<websocketpp::lib::mutex>scoped_lock;
    typedef websocketpp::client<websocketpp::config::asio_tls_client>CLIENT;
    typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context>context_ptr;


    WsHandler() : connected(false), symbolsRecieved( false)
    {
        // set up access channels to only log interesting things
        client.clear_access_channels( websocketpp::log::alevel::all);
        client.set_access_channels( websocketpp::log::alevel::connect);
        client.set_access_channels( websocketpp::log::alevel::disconnect);
        client.set_access_channels( websocketpp::log::alevel::app);

        // Initialize the Asio transport policy
        client.init_asio();

        // Bind the handlers we are using
        client.set_open_handler( websocketpp::lib::bind( &WsHandler::on_open,this,websocketpp::lib::placeholders::_1));
        client.set_close_handler( websocketpp::lib::bind( &WsHandler::on_close,this,websocketpp::lib::placeholders::_1));
        client.set_fail_handler( websocketpp::lib::bind( &WsHandler::on_fail,this,websocketpp::lib::placeholders::_1));
        client.set_tls_init_handler( websocketpp::lib::bind( &WsHandler::on_tls_init, this, websocketpp::lib::placeholders::_1));
        client.set_message_handler( websocketpp::lib::bind( &WsHandler::on_message,this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
    }

    void run( const std::string &uri, size_t delay)
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
        websocketpp::lib::thread stablishStream_thread( &WsHandler::stablishStream, this);

        // Create a thread to log the received data after passing a specified dealy
        websocketpp::lib::thread lazyLogger_thread( &WsHandler::lazyLogger, this, delay);

        asio_thread.detach();
        stablishStream_thread.detach();
        lazyLogger_thread.detach();
    }

    // The open handler will signal that we are ready to start
    void on_open( websocketpp::connection_hdl)
    {
        client.get_alog().write( websocketpp::log::alevel::app, "Connection opened!");

        scoped_lock guard( lock);
        connected = true;
    }

    // The close handler will signal that we should stop
    void on_close( websocketpp::connection_hdl)
    {
        client.get_alog().write( websocketpp::log::alevel::app, "Connection closed!");

        scoped_lock guard(lock);
        connected = false;
    }

    // The fail handler will signal that we should stop
    void on_fail(websocketpp::connection_hdl)
    {
        client.get_alog().write(websocketpp::log::alevel::app, "Connection failed, stopping !");

        scoped_lock guard(lock);
        connected = false;
    }

    // The TLS handler will initial secure connection parameters
    context_ptr on_tls_init( websocketpp::connection_hdl)
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
    void on_message( websocketpp::connection_hdl, CLIENT::message_ptr msg)
    {
        rapidjson::Document document;


        document.Parse( msg->get_payload().c_str());

        if( document.HasParseError())
        {
            std::cout << "Error  : " << document.GetParseError()  << '\n' << "Offset : " << document.GetErrorOffset() << '\n';

            return ;
        }
        else if( document.HasMember("active_symbols"))
        {
            rapidjson::Value &activeSymbols = document["active_symbols"];


            for( auto &symbol : activeSymbols.GetArray())
            {
                if( !symbol.HasMember("symbol"))
                {
                    std::cout << "Error  : " << document.GetParseError()  << '\n' << "Offset : " << document.GetErrorOffset() << '\n';

                    return ;
                }

                rapidjson::Value::ConstMemberIterator itr = symbol.FindMember("symbol");


                if( itr->value.IsString())
                {
                    symbols.push_back( itr->value.GetString());
                }
            }

            scoped_lock guard( lock);
            symbolsRecieved = true;
        }
        else if( document.HasMember("msg_type"))
        {
            rapidjson::Value::ConstMemberIterator itr = document.FindMember("msg_type");


            if(( itr->value.IsString()) && ( !strncmp( itr->value.GetString(), "tick", 4)) && ( document.HasMember("tick")))
            {
                rapidjson::Value& tick = document["tick"];
                TICK_DATA tmpData;


                tmpData.ask = tick.FindMember("ask")->value.GetDouble();
                tmpData.bid = tick.FindMember("bid")->value.GetDouble();
                tmpData.quote = tick.FindMember("quote")->value.GetDouble();
                tmpData.epoch = tick.FindMember("epoch")->value.GetUint();

                ticks.push( tmpData);
            }
        }
    }

    void stablishStream()
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


    bool initTickStream( std::string tickSymbol)
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

    void lazyLogger( size_t delay)
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

    void takeANap( size_t seconds)
    {
        std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::seconds( seconds));
    }

    bool isConnected( size_t waitTime = DEFAULT_WAIT_TIME)
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

    bool activeSymbolsFed()
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

private:

    CLIENT                      client;
    websocketpp::connection_hdl conHandler;
    websocketpp::lib::mutex     lock;
    bool                        connected;
    bool                        symbolsRecieved;
    std::vector<std::string>    symbols;
    std::queue< TICK_DATA>      ticks;
};

int main(int argc, char* argv[])
{
    GumboOutput* output = gumbo_parse("<h1>Hello, World!</h1>");
    // Do stuff with output->root
    gumbo_destroy_output(&kGumboDefaultOptions, output);


    CURL *curl;
    FILE *fp;
    CURLcode res;
    char *url = "https://cdn.ime.co.ir";
    char outfilename[FILENAME_MAX] = "page.html";
    curl = curl_easy_init();
    if (curl)
    {
        fp = fopen(outfilename,"wb");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fp);
    }





















    std::string                 uri = URI;
    std::unique_ptr<WsHandler>  ws(new WsHandler);

    try
    {
        ws->run( uri, DEFAULT_WAIT_TIME);

        while( !ws->activeSymbolsFed())
        {
        }

        ws->initTickStream( "R_50");

        while( true)
        {
        }
    }
    catch( std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }
}
