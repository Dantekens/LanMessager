#include "CommandServer.hpp"
#include <iostream>
#include "../Session/sesionServer.hpp"
#include "../Server/server.hpp"
#include <boost/asio.hpp>

namespace Command_Server {

    //МЕТОДЫ ВАЛИДАТОРА 
    bool Command::Validator::is_id(const std::string& id) {
        if(id.empty()) return false;
        return std::all_of(id.begin(),id.end(),[](char sumbol)
        {
            return std::isdigit(sumbol);
        });
    }
    bool Command::Validator::is_valid_format_command(const std::string& command) {
        if(command.empty()) return false;
        return command[0] == '/';
    }
   
   


    //ОСНОВНЫЕ МЕТОДЫ/КОНСТРУКТОР
    Command::Command(Server* server): server(server) {
        //ИДЕЯ ПРИДУМАТЬ  КОМАНДУ ДЛЯ ПОИСКА ЮЗЕРВО ПО ЛОГИНУ ТОЕСТЬ ЕСЛИ У ЛЮДЕЙ ОДИНАКОВЫ ЛОГИН ТО ИХ вывести
        commands["/list"] = [this](const Atribyte& atribyte)
        {
            if(this->server->users.size() == 0)
            {
                std::cout << "пользователей нет\n";
                return;
            } 

            if(atribyte.is_broadcast)
            {
                std::cout << "\tВсе пользователи\n";
                for(auto& client: this->server->users)
                {
                    auto pointer = client.second->socketClient.remote_endpoint();
                    std::cout << "id " << client.first 
                    <<" \tlogin " << client.second->login
                    << " \tip " << pointer.address().to_string() 
                    << " \tport " << pointer.port()<<std::endl;
                }
            }
            else
            {
                auto client = this->server->users.find(atribyte.id_user);
                if(client == this->server->users.end()) return;
                auto pointer = client->second->socketClient.remote_endpoint();
                std::cout << "id " << client->second->id
                <<" \tlogin " << client->second->login
                << " \tip " << pointer.address().to_string() 
                << " \tport " << pointer.port()<<std::endl;
            }
           
        };
        commands["/send"] = [this](const Atribyte& atribyte)
        {
            if(this->server->users.size() == 0)
            {
                std::cout << "пользователей нет\n";
                return;
            } 

            if(atribyte.is_broadcast)
            {
                for(auto& client: this->server->users)
                {
                    client.second->forming_packet_text("СООБЩЕНИЕ ОТ СЕРВЕРА "+atribyte.messag);
                }
            }
            else
            {
                auto client = this->server->users.find(atribyte.id_user);

                if(client == this->server->users.end()) return;

                client->second->forming_packet_text("СООБЩЕНИЕ ОТ СЕРВЕРА ВАМ "+atribyte.messag);
            }
           
        };
        commands["/kick"] = [this](const Atribyte& atribyte)
        {
            if(this->server->users.size() == 0)
            {
                std::cout << "пользователей нет\n";
                return;
            } 

            if(atribyte.is_broadcast)
            {
                for(auto& client: this->server->users)
                {
                    client.second->forming_packet_text("Вас кикнули");
                }
                this->server->DisconectAllUsers();
            } 
            else {
                auto client = this->server->users.find(atribyte.id_user);

                if(client == this->server->users.end()) return;

                client->second->forming_packet_text("Вас кикнули");
                this->server->DisconectUser(atribyte.id_user);
            } 
        };
    }
    std::vector<std::string> Command::parse_command(std::string& command) {
        std::vector<std::string> token;
        std::string temp_command;
        bool is_qotes = false;
        for(char symbol : command)
        {
            if(symbol == '"') is_qotes = !is_qotes;
            else if(symbol == ' ' && !is_qotes)
            {
                if(!temp_command.empty())
                {
                    token.push_back(temp_command);
                    temp_command.clear();
                }
                
            }
            else temp_command += symbol;
        }
        if(!temp_command.empty()) token.push_back(temp_command);

        return token;
    }
    Command::Instruction Command::parse_to_context(std::string& command) {
        std::vector<std::string> comm = parse_command(command);
        size_t size =  comm.size();
        Command::Instruction instruction;
        Atribyte atribyte;

        if(comm.empty()) return instruction;

        if(!Validator::is_valid_format_command(comm[0]))
        {
            std::cout << "комманда введена не верно \n";
            return instruction; 
        }

        instruction.command = comm[0];

        if(size == 2) atribyte.is_broadcast = comm[1] == "all";
        else if (size == 3)
        {
            atribyte.is_broadcast = comm[1] == "all";
            if(Validator::is_id(comm[2])) atribyte.id_user = std::stoi(comm[2]);
            else atribyte.messag = comm[2];
        }
        else if(size >= 4)
        {
            atribyte.is_broadcast = comm[1] == "all";
            atribyte.id_user = std::stoi(comm[2]);
            atribyte.messag = comm[3];
        }
        
        instruction.atribute = atribyte;
        return instruction;
    }
    void Command::execute(std::string command) {
        Instruction inst = parse_to_context(command);

        if(!Validator::is_check_command_map(inst.command,commands)) 
        {
            std::cout << "Такой команды не существует\n";
            return;
        }
        
        commands[inst.command](inst.atribute);
    }
}



