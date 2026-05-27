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
void Session::readText(int64_t data)
{
    auto self = shared_from_this();
    auto buffer_text = std::make_shared<std::vector<char>>(data);
    boost::asio::async_read(socketClient,boost::asio::buffer(buffer_text->data(),data),[this,self,buffer_text](const boost::system::error_code& er, std::size_t size)
    {
        if(er)
        {
            server.DisconectUser(id);
            server.ReadError("отключился пользователь с id " + id );
            return; 
        }
        std::string text(buffer_text->begin(),buffer_text->end());
        server.sendTextAllUsers(text,id);
        readMessag();
    });
}
void Session::readFileNow(int32_t size_name_file ,int64_t data,int32_t idfile)
{
    auto self = shared_from_this();
    auto namefile = std::make_shared<std::string>();
    namefile->resize(size_name_file);
    boost::asio::async_read(socketClient,boost::asio::buffer(namefile->data(),size_name_file),[this,self,namefile,data,idfile](const boost::system::error_code& er, std::size_t size)
    {
        if(er)
        {
            server.DisconectUser(id);
            server.ReadError("отключился пользователь с id " + id );
            return; 
        }
         std::filesystem::path full_path_to_file = server.path_to_save_file / *namefile;
         ActiveFileInfo file;
         file.bytefile = data;
         file.path_to_file = full_path_to_file;
         file.file = std::make_shared<std::ofstream>(full_path_to_file.c_str(), std::ios::binary);

         if(!file.file->is_open())
         {
             std::cout << "неудалось создать фаил на пути" + full_path_to_file.string() << std::endl;
                readMessag();
                return;
         }

         
         files[idfile] = file;
         readMessag();
    });

}
void Session::readFileNext(int64_t data,int32_t idfile)
{
    auto self = shared_from_this();
    auto it = files.find(idfile);
    if(it == files.end()|| !it->second.file->is_open())
    {
        readMessag();
        return;
    }
    auto buffer = std::make_shared<std::vector<char>>(data);
    boost::asio::async_read(socketClient,boost::asio::buffer(buffer->data(),data), [this,self ,data,idfile,buffer](const boost::system::error_code& er, std::size_t size)
    {
        if(er)
        {
            server.DisconectUser(id);
            server.ReadError("отключился пользователь с id " + id );
            return; 
        }
        auto& file = files[idfile];
        file.file->write(buffer->data(),size);
        file.bytefile -= size;
        if(file.bytefile == 0 )
        {
            file.file->close();
            server.sendFileAllUsers(file.path_to_file,id);
            files.erase(id);
            readMessag();
            return;
        }
        readMessag();
    });
}



 void Session::start()
 {
    std::cout << "новый пользовател id " + id << std::endl;
    readMessag();
 }

Session::Session(boost::asio::ip::tcp::socket&& socket,Server& server,int32_t id) : socketClient(std::move(socket)),server(server),id(id)
{

}
void Session::sendFile(std::filesystem::path path_file)
{
    auto self = shared_from_this();
    
    
}
void Session::sendText(const std::string text)
{
    auto self = shared_from_this();
    auto temp_text = std::make_shared<std::string>(std::move(text));
    auto data = std::make_shared<DataSize>(Type::Text,0,text.size(),0);
     boost::asio::async_write(socketClient,boost::asio::buffer(data.get(),sizeof(*data)),[this,self,data,temp_text](const boost::system::error_code& er, std::size_t size)
     {
        if(er)
        {
             server.DisconectUser(id);
            server.ReadError("отключился пользователь с id " + id );
            return; 
        }
        boost::asio::async_write(socketClient,boost::asio::buffer(*temp_text),[this,self,temp_text](const boost::system::error_code& er, std::size_t size)
        {
            if(er)
            {
                server.DisconectUser(id);
                server.ReadError("отключился пользователь с id " + id );
                return; 
            }
        });
     });
}