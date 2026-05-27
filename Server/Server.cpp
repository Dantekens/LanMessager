#include "server.hpp"
#include <iostream>

void Server::addUsers()
{
    acceptor.async_accept([this](const boost::system::error_code& er,boost::asio::ip::tcp::socket socket)
    {
        if(!er)
        {
            auto user = std::make_shared<Session>(std::move(socket),this,id);
            users[id] = user;
            user->start();
            ++id;
        }
        else
        {
            ReadError("Была попытка подключение");
        }
        addUsers();
    });
}


void Server::sendFileAllUsers(std::filesystem::path file_path,int32_t this_id_user)
{
    for(auto& [user_id,user] : users)
    {
        if(user_id == this_id_user) continue;
        user->sendFile(file_path);
    }

}
void Server::sendTextAllUsers(const std::string& text,int32_t this_id_user)
{
    for(auto& [user_id,user] : users)
    {
        if(user_id == this_id_user) continue;
        user->sendText(text);
    }
}


void Server::sendFileIdUser(std::string& path,int32_t id_user)
{
    auto it = users.find(id_user);
    if(it != users.end())
    {
        it->second->sendFile(path);
    }
}
void Server::sendTextIdUser(const std::string& text,int32_t id_user)
{
     auto it = users.find(id_user);
    if(it != users.end())
    {
        it->second->sendText(text);
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
    id = 1;
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
                ReadError("такого пути нет  хотите создать директорию по данному пути y/n");
                    std::cin >> exis;
                     std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
                    if(exis == 'y'|| exis == 'Y')
                    {
                        std::filesystem::create_directories(path_to_save_file,err);
                        if(err)
                        {
                            ReadError("ошибка"+err.message());
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
                    std::cout << "Все успкшно создано можете  начинать общение "<<std::endl;
                    break;
                }
                    
                else
                {
                    ReadError("это не папка  укажите путь еще раз");
                   continue;
                }
            }
       }                     
    }
void Server::ReadError(std::string&& texterror)
{
    std::cout << "Внимание "+ texterror << std::endl;
}