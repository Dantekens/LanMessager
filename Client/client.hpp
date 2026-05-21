#include "boost/asio.hpp"
#include "EnetworkTypes/MessagType.hpp"
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
    void error(std::string&& er);
    std::unordered_map<int32_t,ActiveFileInfo> Allfile;

    public:

    Client(boost::asio::io_context io);

    bool connect(boost::asio::ip::address addressServet,unsigned short portserver);
    void send_text(std::string& text);
    void send_file(std::string&& path);

    void reverse_read();
    void read_file(std::shared_ptr<DataSize> data);
    void read_text(std::shared_ptr<DataSize> data);
};