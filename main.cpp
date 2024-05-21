#include "postMan.h"


int main(int argc, char* argv[])
{
    PostMan *postMan = new PostMan( CONNECTION_MODE::CONNECTION_MODE_REST, "http://date.jsontest.com/");



    postMan->init();


}
