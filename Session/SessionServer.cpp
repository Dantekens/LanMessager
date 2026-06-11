#include "sesionServer.hpp"
#include "../Server/server.hpp"
#include <iostream>

void Session::readMessag()
{
    
    auto self = shared_from_this();
    auto info = std::make_shared<DataSize>();
    boost::asio::async_read(socketClient,boost::asio::buffer(info.get(),sizeof(*info)),[this,self, info](const boost::system::error_code& er, std::size_t size)
    {
        
        if(er)
        {
            server.read_error("отключился пользователь с id " + id );
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
                    break;
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
           server.read_error("отключился пользователь с id " + id );
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
            server.read_error("отключился пользователь с id " + id );
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
            server.read_error("отключился пользователь с id " + id );
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
        files[idfile] = std::move(file);
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
            server.read_error("отключился пользователь с id " + id );
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

void Session::add_packet_in_qeueue(std::shared_ptr<Packet> packet)
{
    boost::asio::post(strand_server,[this,new_packet = std::move(packet)]()
    {
        this->packet.push(std::move(new_packet));
        if(!sending)
        {
            sending = true;
            send_packet();
        }
    });
    
}
void Session::send_packet()
{
    if(packet.empty())
    {
        sending = false;
        return;
    }
    auto self = shared_from_this();
    std::shared_ptr<Packet> send_packet = packet.front();
    packet.pop();
    boost::asio::async_write(socketClient,boost::asio::buffer(&send_packet->data,sizeof(send_packet->data)),[this,self,send_packet](const boost::system::error_code& er, std::size_t size)
    {
        if(er)
        {
            sending = false;
            server.DisconectUser(id);
            server.read_error("отключился пользователь с id " + id );
            return; 
        }
        std::cout << "Size text "<< send_packet->data.data << " real size text " << send_packet->str.size() << std::endl; 
        boost::asio::async_write(socketClient,boost::asio::buffer(send_packet->str.data(),send_packet->str.size()),[this,self,send_packet](const boost::system::error_code& er, std::size_t size)
        {
            if(er)
            {
                sending = false;
                server.DisconectUser(id);
                server.read_error("отключился пользователь с id " + id );
                return; 
            }
            if(send_packet->stream_file && send_packet->stream_file->is_open() && !send_packet->stream_file->eof() )
            {
                DataSize next_data = send_packet->data;
                std::vector<char> buffer(4096);
                send_packet->stream_file->read(buffer.data(),4096);
                std::streamsize real_byte_read {send_packet->stream_file->gcount()}; 

                if(real_byte_read > 0)
                {
                    next_data.data = real_byte_read;
                    std::vector<char> real_buffer(buffer.begin(),buffer.begin() + real_byte_read);
                    auto next_packet = std::make_shared<Packet>(next_data,real_buffer,send_packet->stream_file);
                    add_packet_in_qeueue(std::move(next_packet));
                }
                else
                send_packet->stream_file->close();
            }
            this->send_packet();
        });
    });
}

 void Session::start()
 {
    std::cout << "новый пользовател id " << id << std::endl;
    readMessag();
 }

 void Session::CloseSocket()
 {
    socketClient.close();
 }
Session::Session(boost::asio::ip::tcp::socket&& socket, boost::asio::io_context& io,Server& server,int id) : socketClient(std::move(socket)),server(server),id(id),strand_server(boost::asio::make_strand(io))
{}
void Session::forming_packet_file(std::filesystem::path path_file)
{
    boost::asio::post(strand_server,[this,path_file,self = shared_from_this()]()
    {
        if(!std::filesystem::exists(path_file) || !std::filesystem::is_regular_file(path_file))
        {
            server.read_error("Пакет файлом не сформирован по причине (путь не верный)");
            return;
        }
        uint64_t size_file = static_cast<uint64_t>(std::filesystem::file_size(path_file));
        std::string name_file = path_file.filename().string();
        DataSize data_file(Type::FileNow,name_file.size(),size_file,id_send_file);
        auto file_stream = std::make_shared<std::ifstream>(path_file.c_str(),std::ios::binary);
        
        auto forming_packet = std::make_shared<Packet>(data_file,name_file);

        add_packet_in_qeueue(std::move(forming_packet));
        ++id_send_file;

        DataSize data(Type::FileNext,0,0,data_file.idFile);
        std::vector<char> buffer(4096);
        file_stream->read(buffer.data(),4096);
        std::streamsize real_byte_real_file = file_stream->gcount();

        if(real_byte_real_file > 0)
        {
            data.data = real_byte_real_file;
            std::vector<char> real_buffer(buffer.begin(),buffer.begin() + real_byte_real_file);

            auto start_packet = std::make_shared<Packet>(data,real_buffer,file_stream);
            add_packet_in_qeueue(std::move(start_packet));
        }
    });
}
void Session::forming_packet_text(const std::string text)
{
        boost::asio::post(strand_server,[this,text, self = shared_from_this()]()
        {
            DataSize data(Type::Text,0,text.size(),0);
            auto temp_packet = std::make_shared<Packet>(data,text);
            add_packet_in_qeueue(std::move(temp_packet));

        });  
}