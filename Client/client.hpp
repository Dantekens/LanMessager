#include "boost/asio.hpp"
#include "EnetworkTypes/MessagType.hpp"
#include <string>
#include <memory>

class Client
{
    private:

    boost::asio::ip::tcp::socket connectserver;
   

    public:

    Client(boost::asio::io_context io);

    bool connect(boost::asio::ip::address addressServet,unsigned short portserver);
    void send_text(std::string& text);
    void send_file(std::string&& path);

    void reverse_read();
    void read_file(std::shared_ptr<DataSize> data);
    void read_text(std::shared_ptr<DataSize> data);
};