#include "client.hpp"
#include <iostream>
#include <vector>



    Client::Client(boost::asio::io_context& io): connectserver(io)
    {
        check_path_file_save();
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
        });
    }
    void Client::error(std::string&& er)
    {
        std::cout << "ВНИМАНИЕ" + er << std::endl;
    }
    void Client::send_text(std::string& text)
    {

        auto data = std::make_shared<DataSize>(Type::Text,0,text.size(),0);
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
            error("Фаил не найден укажите верный путь");
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
                auto buffers = std::make_shared<std::vector<char>>(4096);
                auto fuction = std::make_shared<std::function<void()>>();

                *fuction = [this,self,datafile,fileread,buffers,fuction]()
                {
                    fileread->read(buffers->data(),4096);
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

                            boost::asio::async_write(connectserver,boost::asio::buffer(buffers->data(),datafile->data),[this ,self,fileread,buffers,fuction](const boost::system::error_code& code,std::size_t size)
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
        boost::asio::async_read(connectserver,boost::asio::buffer(namefile.get(),size_name),[this,self,namefile,data,id](const boost::system::error_code& er, std::size_t size)
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
            auto& find_file = Allfile[id];
            find_file.file->write(buffers->data(),size);
            find_file.bytefile -= size;
            if(find_file.bytefile == 0)
            {
                find_file.file->close();
                Allfile.erase(id);

                std::cout << "фаил успешно отправлен" << std::endl;
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