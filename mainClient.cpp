#include <boost/asio.hpp>
#include <string>
#include "Client/client.hpp"
#include <memory>
#include <iostream>
#include <limits>
#include <thread>

void  login(std::shared_ptr<Client> client)
{
    std::string login;
    std::cout << "Введите ваш логин"<<std::endl;
    std::cin >> login;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    client->forming_package_text(login,Type::Login);
}

int main()
{
    std::string path;
    std::string text;
    boost::asio::io_context io;
    auto guar_io = boost::asio::make_work_guard(io);
    auto client = std::make_shared<Client>(io); 
   
    
    client->connect(boost::asio::ip::address{boost::asio::ip::make_address("127.0.0.1")},6009);

   std::thread thr {[&io](){io.run(); }};
    thr.detach();
     
    login(client);

    while (std::getline(std::cin,text))
    {
        if(text.empty()) continue;
        if(text.size() > 6 && text.substr(0,6) == "/file ")
        {
            client->forming_package_send_file(text.substr(6));
            continue;
        }
        else if(text.size() == 6 && text == "/login")
        {
            login(client);
            continue;
        }

        client->forming_package_text(text);
    }
    
    guar_io.reset();
    io.stop();
    if(thr.joinable())
    {
        thr.join();
    }
    

    return 0;
}