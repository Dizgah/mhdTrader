#ifndef CONNECTION_H
#define CONNECTION_H

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

#include "definition.h"


class Connection
{
public:

    explicit Connection()
    {
        url_.clear();
        data_ = std::make_unique< std::string>();
    }

    explicit Connection( std::string_view url):url_(url)
    {
        data_ = std::make_unique< std::string>();
    }

    virtual RESULT init( std::string_view url = "") = 0;

//    virtual RESULT start() = 0;

//    virtual RESULT terminate() = 0;

    void setUrl( std::string_view url)
    {
        url_ = static_cast<std::string>( url);
    }

    std::string_view getUrl() const
    {
        return url_;
    }

    void clearData()
    {
        data_.get()->clear();
    }

    std::string_view getData() const
    {
        return *data_;
    }


    virtual ~Connection()
    {}

protected:

    std::unique_ptr<std::string>    data_;

private:

    std::string                     url_;
};


#endif // CONNECTION_H
