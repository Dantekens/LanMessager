
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
    boost::asio::ip::tcp::acceptor acceptor;


    void addUsers();


    void sendFileAllUsers(std::string& file_path,int this_id_user);
    void sendTextAllUsers(const std::string& text,int this_id_user);
    
    void sendFileIdUser(std::string& path,int id_user);
    void sendTextIdUser(const std::string& text,int id_user);

    void DisconectUser(int id);
    public:

    Server(boost::asio::io_context& io, unsigned short port);
    void ReadError(std::string&& texterror);
    
};