#pragma once

#include <string>
#include <map>
#include <functional>
#include <algorithm>


class Server;

namespace Command_Server {

    class Command {

        private:

        class Validator {
            public:
            static  bool is_id(const std::string& id);
            static  bool is_valid_format_command(const std::string& command);
            template <class KEY,class VALUE>
            static bool is_check_command_map(const std::string& command,std::map<KEY,VALUE>& commands) {
                if(command.empty()) return false;
                return commands.find(command) != commands.end();
            }
        };
        struct Atribyte {
            bool is_broadcast;
            int id_user;
            std::string messag;
            Atribyte(bool is_broadcast,int id,std::string messag): is_broadcast(is_broadcast),id_user(id),messag(messag) {}
            Atribyte(): is_broadcast(false),id_user(-1),messag(""){}
        };
        struct Instruction {
            std::string command;
            Atribyte atribute;
            Instruction(std::string command,Atribyte atribyte):command(command),atribute(atribute){}
            Instruction() = default;
        };
        
        
        std::map<std::string,std::function<void(const Atribyte&)>> commands;

        std::vector<std::string> parse_command(std::string& command);
        Instruction parse_to_context(std::string& command);
        Server* server;

        public:
       
        void execute(std::string command);
        Command(Server* server);
    };
}
