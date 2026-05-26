#pragma once

#include "boost/asio.hpp"
#include <cstdint>
#include <string>
#include <vector>
#include "EnetworkTypes/MessagType.hpp"

class Server;

class Session : public std::enable_shared_from_this<Session>
{
    private:

    std::string login;
    boost::asio::ip::tcp::socket socketClient;
    Server& server;
    int32_t id;

    
    void readMessag();
    void readText(int64_t data);
    void readFileNow(int32_t size_name_file ,int64_t data,int32_t idfile);
    void readFileNext(int64_t data,int32_t idfile);
    void read_login(int64_t data);

    public:

    void CloseSocket();
    void start();
    Session(boost::asio::ip::tcp::socket&& socket,Server& server,int32_t id);
    void sendFile(std::string& path);
    void sendText(std::string&  text);

};