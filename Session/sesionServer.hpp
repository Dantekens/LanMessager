#pragma once

#include "boost/asio.hpp"
#include <cstdint>
#include <string>
#include <vector>
#include "../EnetworkTypes/MessagType.hpp"
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <queue>

namespace Command_Server {
    class Command;
}
class Server;

class Session : public std::enable_shared_from_this<Session>
{
    private:
    
    friend class Command_Server::Command;

    std::unordered_map<int32_t,ActiveFileInfo> files;
    std::queue<std::shared_ptr<Packet>> packet;

    bool sending = false;

    std::string login;
    boost::asio::ip::tcp::socket socketClient;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_server;
    Server& server;
    int id;
    int32_t id_send_file = 0;


    
    void readMessag();
    void readText(int64_t data);
    void readFileNow(int32_t size_name_file ,int64_t data,int32_t idfile);
    void readFileNext(int64_t data,int32_t idfile);
    void read_login(int64_t data);

    void add_packet_in_qeueue(std::shared_ptr<Packet> new_packet);
    void send_packet();

    public:

    void CloseSocket();
    void start();
    Session(boost::asio::ip::tcp::socket&& socket, boost::asio::io_context& io,Server& server,int id);
    void forming_packet_file(std::filesystem::path path_file);
    void forming_packet_text(const std::string text);

};