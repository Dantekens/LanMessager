#include <boost/asio.hpp>
#include <string>
#include "Client/client.hpp"
#include <memory>
#include <iostream>
#include <limits>
#include <thread>

int main()
{
    std::string path;
    std::string text;
    boost::asio::io_context io;
    auto client = std::make_shared<Client>(io); 
   
    client->reverse_read();
    client->connect(boost::asio::ip::address{boost::asio::ip::make_address("1,2,3")},6004);

   std::thread thr {[&io](){io.run(); }};
    thr.detach();


    {
        std::string login;
        std::cout << "Введите ваш логин"<<std::endl;
        std::cin >> login;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
        client->send_text(login);
    }

    while (std::getline(std::cin,text))
    {
        if(text.empty()) continue;
        if(text.size() > 6 && text.substr(0,6) == "/file ")
        {
            client->send_file(text.substr(6));
            continue;
        }

        client->send_text(text);
    }
    

    return 0;
}