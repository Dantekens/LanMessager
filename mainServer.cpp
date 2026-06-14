#include <boost/asio.hpp>
#include <string>
#include "Server/server.hpp"
#include "CommandServer/CommandServer.hpp"
#include <memory>
#include <iostream>
#include <limits>
#include <thread>


int main() 
{
    std::string command;
    boost::asio::io_context io;
   auto io_res =  boost::asio::make_work_guard(io);
    auto server = std::make_shared<Server>(io,6009);
    std::thread  th {[&io](){io.run();}};
    auto command_to_server = std::make_shared<Command_Server::Command>(server.get());
      
    
    while (std::getline(std::cin,command))
    {
       command_to_server->execute(command);
    }
    io_res.reset();
    io.stop();
    th.join();
    return 0;
}