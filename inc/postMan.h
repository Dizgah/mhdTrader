#ifndef POSTMAN_H
#define POSTMAN_H


#include <iostream>
#include <fstream>
#include <cstdlib>
#include<chrono>
#include<thread>
#include <vector>
#include <string>
#include <string_view>
#include <queue>
#include <list>
#include <memory>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/ostreamwrapper.h>

#include "definition.h"
#include "restManager.h"
#include "websocketManager.h"


class PostMan
{
public:

    PostMan()
    {
        setMode( CONNECTION_MODE::CONNECTION_MODE_REST);

        rest_ = std::make_unique< RestManager>();
    }

    PostMan( CONNECTION_MODE mode)
    {
        setMode( mode);

        switch( mode)
        {
            case CONNECTION_MODE::CONNECTION_MODE_REST:

                rest_ = std::make_unique< RestManager>();
            break;

            case CONNECTION_MODE::CONNECTION_MODE_WEBSOCKET:

//                ws_ = std::make_unique< WebsocketManager>();
            break;


            default:

            break;
        }
    }

    PostMan( CONNECTION_MODE mode, std::string_view url)
    {
        setMode( mode);

        switch( mode)
        {
            case CONNECTION_MODE::CONNECTION_MODE_REST:

                rest_ = std::make_unique< RestManager>( url);
            break;

            case CONNECTION_MODE::CONNECTION_MODE_WEBSOCKET:

//                ws_ = std::make_unique< WebsocketManager>( url);
            break;


            default:

            break;
        }

    }

    void setMode( CONNECTION_MODE mode)
    {
        mode_ = mode;
    }

    CONNECTION_MODE getMode() const
    {
        return mode_;
    }

    RESULT init( std::string_view url = "")
    {
        switch( getMode())
        {
            case CONNECTION_MODE::CONNECTION_MODE_REST:

                rest_->init( url);
            break;

            case CONNECTION_MODE::CONNECTION_MODE_WEBSOCKET:

            break;


            default:

            break;
        }



        return RESULT::RESULT_SUCCESS;
    }


    virtual ~PostMan()
    {}

private:

    std::unique_ptr< RestManager>       rest_;
    std::unique_ptr< WebsocketManager>  ws_;
    CONNECTION_MODE                     mode_;
};

#endif // POSTMAN_H
