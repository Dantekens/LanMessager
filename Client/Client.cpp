#include "client.hpp"
#include <iostream>



    Client::Client(boost::asio::io_context io): connectserver(io){}

    bool Client::connect(boost::asio::ip::address addressServet,unsigned short portserver)
    {
        connectserver.async_connect(boost::asio::ip::tcp::endpoint(addressServet,portserver),[this](const boost::system::error_code& er)
        {
            if(er)
            {
                error("сервер отключен");
                return;
            }
        });
    }
    void Client::error(std::string&& er)
    {
        std::cout << "ВНИМАНИЕ" + er << std::endl;
    }
    void Client::send_text(std::string& text)
    {

        auto data = std::make_shared<DataSize>(Type::Text,0,text.size());
        auto textHeap = std::make_shared<std::string>(std::move(text));

        boost::asio::async_write(connectserver,boost::asio::buffer(data.get(),sizeof(*data)),[this,data,textHeap](const boost::system::error_code& er,std::size_t size)
        {
            if(er)
            {
                error("соединение разорвано");
                return;
            }

            boost::asio::async_write(connectserver,boost::asio::buffer(*textHeap),[this,textHeap](const boost::system::error_code& er,std::size_t size)
            {
                if(er)
                {
                    error("соединение разорвано");
                    return;
                }
            });
        });

       
    }
    void Client::send_file(std::string&& path)
    {
        std::filesystem::path path_file{path};
        if(!std::filesystem::exists(path_file) || !std::filesystem::is_regular_file(path_file)) 
        {
            error("Фаил не найден укажите верный пкть");
            return;
        }     
         
        int64_t sizefile = static_cast<int64_t>(std::filesystem::file_size(path_file));

        std::string filename = path_file.filename().string(); 
        auto data = std::make_shared<DataSize>(Type::FileNow,filename.size(),sizefile,idsendfile);
        auto self = shared_from_this();
        auto fileread = std::make_shared<std::ifstream>(path,std::ios::binary);

        ++idsendfile;
        boost::asio::async_write(connectserver,boost::asio::buffer(data.get(),sizeof(*data)),[this,filename,self,data,fileread](const boost::system::error_code& code,std::size_t size)
        {
            if(code)
            {
                error("соединение разорвано");
                return;
            }

            boost::asio::async_write(connectserver,boost::asio::buffer(filename),[this,filename,data, self,fileread](const boost::system::error_code& code,std::size_t size)
            {
                if(code)
                {
                    error("соединение разорвано");
                    return;
                }
                auto datafile = std::make_shared<DataSize>(Type::FileNext,0,0,data->idFile);
                auto buffers = std::make_shared<char[]>(4096);
                auto fuction = std::make_shared<std::function<void()>>();

                *fuction = [this,self,datafile,fileread,buffers,fuction]()
                {
                    fileread->read(buffers.get(),4096);
                    std::streamsize realbyte{fileread->gcount()};
                    
                    if(realbyte > 0)
                    {
                        datafile->data = realbyte;
                        boost::asio::async_write(connectserver,boost::asio::buffer(datafile.get(),sizeof(*datafile)),[this,self,datafile,fileread,buffers,fuction](const boost::system::error_code& code,std::size_t size)
                        {
                            if(code)
                            {
                                error("соединение разорвано");
                                return;
                            }

                            boost::asio::async_write(connectserver,boost::asio::buffer(buffers.get(),datafile->data),[this ,self,fileread,buffers,fuction](const boost::system::error_code& code,std::size_t size)
                            {
                                if(code)
                                {
                                error("соединение разорвано");
                                    return;
                                }
                                (*fuction)();
                            });
                        });

                        
                    }
                    else
                       error("Фаил успешно отправлен");
                    
                };

                (*fuction)();
                
            });
        });

    }

    void Client::reverse_read()
    {
        

    }
    void Client::read_file(std::shared_ptr<DataSize> data)
    {

    }
    void Client::read_text(std::shared_ptr<DataSize> data)
    {

    }