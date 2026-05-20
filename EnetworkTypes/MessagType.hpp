#define once
#include <cstdint>


enum class Type : uint8_t
{
    Text,
    File
};

#pragma pack(push,1)

struct DataSize
{
    uint8_t type;
    int32_t nameFile;
    int64_t data;
    DataSize(Type typemessag,int32_t namefile,int64_t data) : type(static_cast<uint8_t>(typemessag)), nameFile(namefile), data(data){}
    DataSize(uint8_t typemessag,int32_t namefile,int64_t data) : type(typemessag), nameFile(namefile), data(data){}
    DataSize() : type(0), nameFile(0), data(0){}
};

#pragma pack(pop)