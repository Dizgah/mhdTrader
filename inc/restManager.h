#ifndef RESTMANAGER_H
#define RESTMANAGER_H

#include "connection.h"

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
#include <string.h>
#include <optional>

#include <curl/curl.h>

#include "definition.h"
#include "connection.h"


class RestManager : public Connection
{
public:
    explicit RestManager()
    {
        headers         = NULL;
        responseCode    = 0;
        memset( errbuf, 0, CURL_ERROR_SIZE);
    }

    explicit RestManager( std::string_view url):Connection( url)
    {
        headers         = NULL;
        responseCode    = 0;
        memset( errbuf, 0, CURL_ERROR_SIZE);
    }

    static std::size_t callback(
            const char* in,
            std::size_t size,
            std::size_t num,
            std::string* out)
    {
        const std::size_t totalBytes(size * num);
        out->append(in, totalBytes);
        return totalBytes;
    }

    RESULT init( std::string_view url = "")
    {
        if( !url.empty())
        {
            setUrl( url);
        }
        else if( getUrl().empty())
        {
            return RESULT::RESULT_DATA_INVALID;
        }

        try
        {
            curl = curl_easy_init();

            // Set remote URL.
            curl_easy_setopt( curl, CURLOPT_URL, getUrl().data());

            // Don't bother trying IPv6, which would increase DNS resolution time.
            curl_easy_setopt( curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

            // Don't wait forever, time out after 10 seconds.
            curl_easy_setopt( curl, CURLOPT_TIMEOUT, 10);

            // Follow HTTP redirects if necessary.
            curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 1L);

            // Hook up data handling function.
            curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, RestManager::callback);

            return RESULT::RESULT_SUCCESS;
        }
        catch( ...)
        {
            std::cout << "Exception raised!" << std::endl;
        }


        return RESULT::RESULT_SUCCESS;
    }

    std::optional< std::string &>sendRequest( std::string_view data)
    {
        try
        {
            CURLcode resp = CURLE_FAILED_INIT;


            // Hook up data container (will be passed as the last parameter to the
            // callback handling function).  Can be any pointer type, since it will
            // internally be passed as a void pointer.
            curl_easy_setopt( curl, CURLOPT_WRITEDATA, data.data());

            curl_easy_setopt( curl, CURLOPT_ERRORBUFFER, errbuf);

            // Run our HTTP GET command, capture the HTTP response code, and clean up.
            resp = curl_easy_perform(curl);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
            curl_easy_cleanup(curl);

            if( resp != CURLE_OK)
            {
                std::cout << "Libcurl Err =" << resp << std::endl;

                if( strlen( errbuf))
                {
                    std::cout << curl_easy_strerror(resp) << std::endl;
                    std::cout << errbuf << std::endl;
                }
            }

            switch( responseCode)
            {
                case 200:

                    std::cout << "\nGot successful response from " << getUrl().data() << std::endl;
                break;


                default:

                    std::cout << "Couldn't GET from " << getUrl().data() << " - exiting" << std::endl;
                break;
            }


            return RESULT::RESULT_SUCCESS;
        }
        catch( ...)
        {
            std::cout << "Exception raised!" << std::endl;
        }


        return RESULT::RESULT_UNDEFINED;
    }

    virtual ~RestManager()
    {
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

private:

    CURL        *curl;
    FILE        *fp;
    CURLcode    res;
    long        responseCode;
    char        errbuf[CURL_ERROR_SIZE];
    struct      curl_slist *headers;
};

#endif // RESTMANAGER_H
