#define once
#include <cstdint>
#include <fstream>
#include <filesystem>
#include <string>


enum class Type : uint8_t
{
    Text,
    FileNow,
    FileNext,
    Login
};

struct ActiveFileInfo
{
    std::shared_ptr<std::ofstream> file;
    int64_t bytefile = 0;
    std::filesystem::path path_to_file;
};


#pragma pack(push,1)

struct DataSize
{
    uint8_t type;
    int32_t nameFile;
    int64_t data;
    int32_t idFile = 0;
    int32_t idUersToSend = -1;
    DataSize(Type typemessag, int32_t namefile, int64_t data, int32_t idfile) : type(static_cast<uint8_t>(typemessag)), nameFile(namefile), data(data), idFile(idfile) {}
    DataSize(uint8_t typemessag, int32_t namefile, int64_t data, int32_t idfile) : type(typemessag), nameFile(namefile), data(data), idFile(idfile) {}
    DataSize() : type(0), nameFile(0), data(0), idFile(0) {}
};

#pragma pack(pop)

struct Packet
{
    DataSize data;
    std::vector<char> str; 
    std::shared_ptr<std::ifstream> stream_file = nullptr;

    Packet(DataSize data,std::vector<char> str) : data(data), str(std::move(str)){}
    Packet(DataSize data,std::string str) : data(data), str(str.begin(),str.end()){}
    Packet(DataSize data,std::vector<char> str,std::shared_ptr<std::ifstream> stream_file) : data(data), str(std::move(str)),stream_file(stream_file){}
};