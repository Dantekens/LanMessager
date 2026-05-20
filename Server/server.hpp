
#pragma once

#include "boost/asio.hpp"
#include "EnetworkTypes/MessagType.hpp"
#include <string>
#include <memory>
#include <unordered_map>
#include <cstdint>
#include <Session/sesionServer.hpp>

class Server
{
    private:

    friend class Session;


    std::unordered_map<int32_t,std::shared_ptr<Session>> users;
    static int32_t id;
    boost::asio::io_context& io;


    void addUsers();


    void sendFileAllUsers(std::string& filename);
    void sendTextAllUsers(std::shared_ptr<std::string> text);

    void sendFileIdUser(std::string& filename);
    void sendTextIdUser(std::shared_ptr<std::string> text);

    public:

    Server(boost::asio::io_context& io, unsigned short port);
    void ReadError(std::string&& texterror);
    
};