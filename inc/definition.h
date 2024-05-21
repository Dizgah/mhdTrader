#ifndef DEFINITION_H
#define DEFINITION_H

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


enum class RESULT
{
    RESULT_DATA_INVALID,
    RESULT_CONNECTION_BROKEN,


    RESULT_SUCCESS,

    RESULT_UNDEFINED
};

enum class CONNECTION_MODE
{
    CONNECTION_MODE_REST,
    CONNECTION_MODE_WEBSOCKET,

    CONNECTION_MODE_UNDEFINED
};














#endif // DEFINITION_H
