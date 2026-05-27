
#pragma once

#include "boost/asio.hpp"
#include "EnetworkTypes/MessagType.hpp"
#include <string>
#include <memory>
#include <unordered_map>
#include <cstdint>
#include <Session/sesionServer.hpp>
#include <filesystem>
#include <fstream>

class Server
{
    private:

    friend class Session;


    std::unordered_map<int32_t,std::shared_ptr<Session>> users;
    
    static int32_t id;
    boost::asio::io_context& io;
    boost::asio::ip::tcp::acceptor acceptor;
     std::filesystem::path path_to_save_file;

    void addUsers();


    void sendFileAllUsers(std::filesystem::path ile_path,int32_t this_id_user);
    void sendTextAllUsers(const std::string& text,int32_t this_id_user);
    
    void sendFileIdUser(std::string& path,int32_t id_user);
    void sendTextIdUser(const std::string& text,int32_t id_user);

    void DisconectUser(int32_t id);

    public:

    void check_path_file_save();
    Server(boost::asio::io_context& io, unsigned short port);
    void ReadError(std::string&& texterror);
    
};