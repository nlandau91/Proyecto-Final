#ifndef ENUMPARSER_H
#define ENUMPARSER_H

#include <string>
#include <map>

template <typename T>
class EnumParser
{
public:
    EnumParser(){};

    T parse_enum(const std::string &value)
    {
        typename std::map<std::string, T>::const_iterator iValue = enumMap.find(value);
        if (iValue  == enumMap.end())
            throw std::runtime_error("");
        return iValue->second;
    }

    void add_enum(T enum_value, std::string enum_name)
    {
        enumMap[enum_name] = enum_value;
    }

private:
    std::map <std::string, T> enumMap;
};

#endif // ENUMPARSER_H
