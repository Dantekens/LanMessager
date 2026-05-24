#include "boost/asio.hpp"
#include "../EnetworkTypes/MessagType.hpp"
#include <string>
#include <memory>
#include <filesystem>
#include <fstream>
#include <unordered_map>

class Client : std::enable_shared_from_this<Client>
{
    private:
    
    boost::asio::ip::tcp::socket connectserver;
    uint32_t idsendfile = 0;

    std::unordered_map<int32_t,ActiveFileInfo> Allfile;
    std::filesystem::path path_to_save_file;

    void check_path_file_save();
    void error(std::string&& er);

    public:

    Client(boost::asio::io_context& io);

    void connect(boost::asio::ip::address addressServet,unsigned short portserver);
    void send_text(std::string& text);
    void send_file(std::string&& path);

    void reverse_read();
    void read_file_now(uint32_t size_name,uint64_t data,uint32_t id);
    void read_file_next(uint64_t data,uint32_t id);
    void read_text(uint64_t data);
};