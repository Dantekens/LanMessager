#include <boost/asio.hpp>
#include <string>
#include "Server/server.hpp"
#include <memory>
#include <iostream>
#include <limits>
#include <thread>


int main() 
{
    boost::asio::io_context io;
    auto server = std::make_shared<Server>(&io,6009);
    

    return 0;
}