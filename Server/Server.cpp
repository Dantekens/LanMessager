#include "server.hpp"
#include <iostream>


void Server::addUsers()
{
    acceptor.async_accept(boost::asio::make_strand(io),[this](const boost::system::error_code& er,boost::asio::ip::tcp::socket socket)
    {
        if(!er)
        {
            auto user = std::make_shared<Session>(std::move(socket),io,*this,id);
            users[id] = user;
            user->start();
            ++id;
        }
        else
        {
            read_error("Была попытка подключение");
        }
        addUsers();
    });
}

void Server::sendFileAllUsers(std::filesystem::path file_path,int this_id_user)
{
    std::lock_guard<std::mutex> lokers(loger);
    for(auto& [user_id,user] : users)
    {
        if(user_id == this_id_user) continue;
        user->forming_packet_file(file_path);
    }

}
void Server::sendTextAllUsers(const std::string text,int this_id_user)
{
    for(auto& [user_id,user] : users)
    {
        if(user_id == this_id_user) continue;
        user->forming_packet_text(text);
    }
}

void Server::sendFileIdUser(std::filesystem::path path,int id_user)
{
    std::lock_guard<std::mutex> lokers(loger);
    auto it = users.find(id_user);
    if(it != users.end())
    {
        it->second->forming_packet_file(path);
    }
}
void Server::sendTextIdUser(const std::string text,int id_user)
{
    std::lock_guard<std::mutex> lokers(loger);
     auto it = users.find(id_user);
    if(it != users.end())
    {
        it->second->forming_packet_text(text);
    }
}
void Server::DisconectUser(int32_t id)
{
    auto it = users.find(id);
    if(it != users.end())
    {
        it->second->CloseSocket();
        boost::asio::post(io,[this,id]()
        {
            users.erase(id);
        });
    }
}

Server::Server(boost::asio::io_context& io, unsigned short port) : io(io), acceptor(boost::asio::ip::tcp::acceptor(io,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port)))
{
    check_path_file_save();
    addUsers();
}
void Server::check_path_file_save()
    {
       std::string filepath;

       while(true)
       {
            std::cout << "Введите путь для сохранения фалов "<<std::endl;
            std::getline(std::cin,filepath);
            path_to_save_file = filepath;
            if(!std::filesystem::exists(filepath))
            {
                    char exis;
                    std::error_code err;
                    read_error("такого пути нет  хотите создать директорию по данному пути y/n");
                    std::cin >> exis;
                     std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
                    if(exis == 'y'|| exis == 'Y')
                    {
                        std::filesystem::create_directories(path_to_save_file,err);
                        if(err)
                        {
                            read_error("ошибка"+err.message());
                            continue;
                        } 
                        break;
                    }
                    else
                    continue; 
            }
            else
            {
                if(std::filesystem::is_directory(path_to_save_file))
                {
                    std::cout << "Все успешно сервер начал работу"<<std::endl;
                    break;
                }
                    
                else
                {
                    read_error("это не папка  укажите путь еще раз");
                   continue;
                }
            }
       }                     
    }
void Server::read_error(std::string&& texterror)
{
    std::lock_guard<std::mutex> lokers(loger);
    std::cout << "ВНИМАНИЕ "+ texterror << std::endl;
}