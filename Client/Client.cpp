#include "client.hpp"
#include <iostream>
#include <vector>



Client::Client(boost::asio::io_context& io): connectserver(io)
{
        check_path_file_save();
}
void  Client::add_packet_in_queue(std::shared_ptr<Packet> packet)
{
   
    this->packet.push(std::move(packet));
    if(seding)
    {
        seding = true;
        send_packet();
    }
}
void Client::send_packet()
{
    if(packet.empty())
    {
        return;
        seding = false;
    }
    std::shared_ptr<Packet> ready_packet = packet.front();
    packet.pop();
    auto self = shared_from_this();

    boost::asio::async_write(connectserver,boost::asio::buffer(&ready_packet->data,sizeof(ready_packet->data)),[this,self,ready_packet](const boost::system::error_code& er,std::size_t size)
    {
        if(er)
        {
            seding = false;
            error("соединение разорвано");
            return;
        }

        boost::asio::async_write(connectserver,boost::asio::buffer(ready_packet->str.data(),ready_packet->str.size()),[this,self,ready_packet](const boost::system::error_code& er,std::size_t size)
        {
            if(er)
            {
                seding = false;
                error("соединение разорвано");
                 return;
            }

            if(ready_packet->stream_file && ready_packet->stream_file->is_open()&&!ready_packet->stream_file->eof())
            {
                DataSize datafile = ready_packet->data;
                std::vector<char> buffers(4096);
                ready_packet->stream_file->read(buffers.data(),4096);
                std::streamsize realbyte{ready_packet->stream_file->gcount()};
                    
                if(realbyte > 0)
                {
                    datafile.data = realbyte;
                   std::vector<char> real_buffer_file{buffers.begin(),buffers.begin()+realbyte};
                    auto packet = std::make_shared<Packet>(datafile,real_buffer_file,ready_packet->stream_file);

                     add_packet_in_queue(std::move(packet));
                }
                else
                ready_packet->stream_file->close();
            }
        });
    });

}

    void Client::check_path_file_save()
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
                error("такого пути нет  хотите создать директорию по данному пути y/n");
                    std::cin >> exis;
                     std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
                    if(exis == 'y'|| exis == 'Y')
                    {
                        std::filesystem::create_directories(path_to_save_file,err);
                        if(err)
                        {
                            error("ошибка"+err.message());
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
                    error("это не папка  укажите путь еще раз");
                   continue;
                }
            }
       }                     
    }

    void Client::connect(boost::asio::ip::address addressServet,unsigned short portserver)
    {
        connectserver.async_connect(boost::asio::ip::tcp::endpoint(addressServet,portserver),[this](const boost::system::error_code& er)
        {
            if(er)
            {
                error("сервер отключен");
                return;
            }
            reverse_read();
        });
    }
    void Client::error(std::string&& er)
    {
        std::cout << "ВНИМАНИЕ" + er << std::endl;
    }
    void Client::forming_package_text(std::string text)
    {
        DataSize data(Type::Text,0,text.size(),0);
        auto self = shared_from_this();
        auto packet = std::make_shared<Packet>(data,text);
        add_packet_in_queue(std::move(packet));  
    }
    void Client::forming_package_send_file(std::string&& path)
    {
        std::filesystem::path path_file{path};
        if(!std::filesystem::exists(path_file) || !std::filesystem::is_regular_file(path_file)) 
        {
            error("Фаил не найден укажите верный путь");
            return;
        }     
         
        int64_t sizefile = static_cast<int64_t>(std::filesystem::file_size(path_file));

        std::string filename = path_file.filename().string(); 
        DataSize data (Type::FileNow,filename.size(),sizefile,idsendfile);
        auto self = shared_from_this();
        auto fileread = std::make_shared<std::ifstream>(path.c_str(),std::ios::binary);
        auto packet = std::make_shared<Packet>(data,filename);

        add_packet_in_queue(std::move(packet));


            DataSize datafile (Type::FileNext,0,0,data.idFile);
            
                std::vector<char> buffers(4096);
                fileread->read(buffers.data(),4096);
                std::streamsize realbyte{fileread->gcount()};
                    
                if(realbyte > 0)
                {
                    datafile.data = realbyte;
                   std::vector<char> real_buffer_file{buffers.begin(),buffers.begin()+realbyte};
                    auto packet = std::make_shared<Packet>(datafile,real_buffer_file,fileread);

                     add_packet_in_queue(std::move(packet));
                }
                
            
        ++idsendfile;
    }

    void Client::reverse_read()
    {
        auto self = shared_from_this();
        auto data = std::make_shared<DataSize>();
        boost::asio::async_read(connectserver,boost::asio::buffer(data.get(),sizeof(*data)),[this,self,data](const boost::system::error_code& er, std::size_t size)
        {
            if(er)
            {
                error("ошибка подключения");
                return;
            }
            switch(data->type)
            {
                case (uint8_t)Type::Text:
                    read_text(data->data);
                break;

                case (uint8_t)Type::FileNow:
                    read_file_now(data->nameFile,data->data,data->idFile);
                break;

                case (uint8_t)Type::FileNext:
                    read_file_next(data->data,data->idFile);
                break;
            }

        });

    }
    void Client::read_file_now(uint32_t size_name,uint64_t data,uint32_t id)
    {
        auto namefile = std::make_shared<std::string>();
        namefile->resize(size_name);
        auto self = shared_from_this();
        boost::asio::async_read(connectserver,boost::asio::buffer(namefile->data(),size_name),[this,self,namefile,data,id](const boost::system::error_code& er, std::size_t size)
        {
            if(er)
            {
                reverse_read();
                error("ошибка подключения");
                return;
            }
            std::filesystem::path full_path_save_file = path_to_save_file / *namefile;
            ActiveFileInfo info;
            info.bytefile = data;
            info.file = std::make_shared<std::ofstream>(full_path_save_file,std::ios::binary);

            if(!info.file->is_open())
            {
                std::cout << "неудалось создать фаил на пути" + full_path_save_file.string() << std::endl;
                reverse_read();
                return;
            }
            Allfile[id] = info;
             reverse_read();
        });

    }
    void Client::read_file_next(uint64_t data,uint32_t id)
    {
        auto  self = shared_from_this();
        auto it = Allfile.find(id);
        if(it == Allfile.end() || !it->second.file->is_open())
        {
            reverse_read();
            return;
        }
        auto buffers = std::make_shared<std::vector<char>>(data);
        boost::asio::async_read(connectserver,boost::asio::buffer(buffers->data(),data),[this,id,data,self,buffers](const boost::system::error_code &er,std::size_t size)
        {
            if(er)
            {
                 error("соединение разорвано");
                return;
            }

            auto it = Allfile.find(id);
            if(it != Allfile.end() && it->second.file->is_open())
            {
                auto& find_file = it->second;
                find_file.file->write(buffers->data(),size);
                find_file.bytefile -= static_cast<int64_t>(size);
                if(find_file.bytefile <= 0)
                {
                    find_file.file->close();
                    Allfile.erase(id);

                    std::cout << "фаил успешно принят" << std::endl;
                }
            }
            reverse_read();

        });

    }
    void Client::read_text(uint64_t data)
    {
        auto buffertext = std::make_shared<std::vector<char>>(data);
       
        auto self = shared_from_this();

        boost::asio::async_read(connectserver,boost::asio::buffer(buffertext->data(),data),[this,self,buffertext](const boost::system::error_code& er, std::size_t size)
        {
             if(er)
            {
                error("ошибка подключения");
                return;
            }

            std::string text{buffertext->begin(),buffertext->end()};
            std::cout << text <<std::endl;
            reverse_read();
        });

    }