#include "sesionServer.hpp"
#include "../Server/server.hpp"
#include <iostream>

 std::string login;

Server& server;
int32_t id;

    
void Session::readMessag()
{
    auto self = shared_from_this();
    auto info = std::make_shared<DataSize>();
    boost::asio::async_read(socketClient,boost::asio::buffer(info.get(),sizeof(*info)),[this,self, info](const boost::system::error_code& er, std::size_t size)
    {
        if(er)
        {
            server.ReadError("отключился пользователь с id " + id );
            return;
        }
        switch(info->type)
        {
            case (uint8_t)Type::Text:
                    readText(info->data);
                break;

                case (uint8_t)Type::FileNow:
                    readFileNow(info->nameFile,info->data,info->idFile);
                break;

                case (uint8_t)Type::FileNext:
                    readFileNext(info->data,info->idFile);
                case (uint8_t)Type::Login:
                    read_login(info->data);
                break;
        }
    });
}
void Session::read_login(int64_t data)
{
    auto self = shared_from_this();
    auto logi = std::make_shared<std::vector<char>>(data);
    boost::asio::async_read(socketClient,boost::asio::buffer(logi->data(),data),[this,self,logi](const boost::system::error_code& er, std::size_t size)
    {
        if(er)
        {
            server.DisconectUser(id);
           server.ReadError("отключился пользователь с id " + id );
            return; 
        }
        if(!login.empty())
        {
            std::string temp_login = login; 
            login.assign(logi->begin(),logi->end());
            server.sendTextAllUsers("пользователь с логином " + temp_login + " поменял  логин теперь у него логин " + login,id);
            readMessag();
        }

        login.assign(logi->begin(),logi->end());

         server.sendTextAllUsers("пользователь с id " + std::to_string(id) + " и логином " + login + " присоединился",id);

        readMessag();

    });
}

    
 void Session::start()
 {
    std::cout << "новый пользовател id " + id << std::endl;
    readMessag();
 }

Session::Session(boost::asio::ip::tcp::socket&& socket,Server& server,int32_t id) : socketClient(std::move(socket)),server(server),id(id),login(nullptr)
{

}
void Session::sendFile(std::string& path){}
void Session::sendText(std::shared_ptr<char> text){}