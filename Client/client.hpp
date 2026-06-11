#include "boost/asio.hpp"
#include "../EnetworkTypes/MessagType.hpp"
#include <string>
#include <memory>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <queue>
#include <mutex>




class Client : std::enable_shared_from_this<Client>
{
    private:
    
    boost::asio::ip::tcp::socket connectserver;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_client;


    uint32_t idsendfile = 0;
    bool seding = false;

    std::unordered_map<int32_t,ActiveFileInfo> Allfile;
    std::filesystem::path path_to_save_file;
    std::queue<std::shared_ptr<Packet>> packet;

    void check_path_file_save();
    void error(std::string&& er);
    void send_packet();
    void add_packet_in_queue(std::shared_ptr<Packet> packet);

    public:

    Client(boost::asio::io_context& io);

    void connect(boost::asio::ip::address addressServet,unsigned short portserver);
    void forming_package_text(std::string text,Type type_text = Type::Text);
    void forming_package_send_file(std::string&& path);

    void reverse_read();
    void read_file_now(uint32_t size_name,uint64_t data,uint32_t id);
    void read_file_next(uint64_t data,uint32_t id);
    void read_text(uint64_t data);
};