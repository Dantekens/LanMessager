#include <boost/asio.hpp>
#include <string>
#include "Server/server.hpp"
#include <memory>
#include <iostream>
#include <limits>
#include <thread>
#include "PoolThread/PoolThread.hpp"

int main() 
{
    std::string command;
    boost::asio::io_context io;

    auto io_res =  boost::asio::make_work_guard(io);
    auto server = std::make_shared<Server>(io,6009);

    PoolThread pool_thread(io,5);
    

    while (std::getline(std::cin,command))
    {
       
    }
    io_res.reset();
    
    return 0;
}