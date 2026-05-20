#include "client.hpp"




    Client::Client(boost::asio::io_context io): connectserver(io){}

    bool Client::connect(boost::asio::ip::address addressServet,unsigned short portserver);
    void Client::send_text(std::string& text);
    void Client::send_file(std::string&& path);

    void Client::reverse_read();
    void Client::read_file(std::shared_ptr<DataSize> data);
    void Client::read_text(std::shared_ptr<DataSize> data);