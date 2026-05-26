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


void Server::sendFileAllUsers(std::string& file_path,int this_id_user)
{
    for(auto& [user_id,user] : users)
    {
        if(user_id == this_id_user) continue;
        user->sendFile(file_path);
    }

}
void Server::sendTextAllUsers(const std::string& text,int this_id_user)
{
    for(auto& [user_id,user] : users)
    {
        if(user_id == this_id_user) continue;
        user->sendText(text);
    }
}


void Server::sendFileIdUser(std::string& path,int id_user)
{
    for(auto& [user_id,user] : users)
    {
        if(user_id == id_user) user->sendFile(path);;
    }

}
void Server::sendTextIdUser(const std::string& text,int id_user)
{
    for(auto& [user_id,user] : users)
    {
        if(user_id == id_user) user->sendText(text);;
    }
}

  
void Server::DisconectUser(int id)
{
    auto& user = users[id];
    user->CloseSocket();
    users.erase(id);
    
}

Server::Server(boost::asio::io_context& io, unsigned short port) : io(io), acceptor(boost::asio::ip::tcp::acceptor(io,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port)))
{
    addUsers();
}
void Server::ReadError(std::string&& texterror)
{
    std::cout << "Внимание "+ texterror << std::endl;
}