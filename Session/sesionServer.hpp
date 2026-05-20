#pragma once

#include "boost/asio.hpp"
#include <cstdint>
#include <string>
#include "EnetworkTypes/MessagType.hpp"

class Server;

class Session
{
    private:

    std::string& path;
    boost::asio::ip::tcp::socket socketClient;
    Server& server;
    int32_t id;

    void readMessag();
    void readText(std::shared_ptr<DataSize> data);
    void readFile(std::shared_ptr<DataSize> data);

    public:


    Session(boost::asio::ip::tcp::socket socket,Server& server,int32_t id, std::string& path);
    void sendFile();
    void sendText(std::shared_ptr<std::string> text);

};