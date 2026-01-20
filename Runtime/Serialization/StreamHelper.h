#pragma once
#include "AssetHeader.h"
#include <fstream>
#include <vector>
#include <string>

namespace EngineCore
{
    class StreamHelper
    {
    public:
        // 写pod data
        template<typename T>
        static void Write(std::ofstream& out, const T& value)
        {
            out.write(reinterpret_cast<const char*>(&value), sizeof(T));
        }

        // 读pod data
        template<typename T>
        static void Read(std::ifstream& in, T& value)
        {
            in.read(reinterpret_cast<char*>(&value), sizeof(T));
        };

        template<typename T>
        static void WriteVector(std::ofstream& out, const std::vector<T>& vec)
        {
            uint32_t size = (uint32_t) vec.size();
            out.write((char*)&size, sizeof(uint32_t));
            if(size > 0)out.write((char*)vec.data(), size * sizeof(T));
        }

        // 直接整段copy
        template<typename T>
        static void ReadVector(std::ifstream& in, std::vector<T>& vec)
        {
            uint32_t size = 0;
            in.read((char*)&size, sizeof(uint32_t));
            vec.resize(size);
            if(size > 0)in.read((char*)vec.data(), sizeof(T) * size);
        }

        static void ReadString(std::ifstream& in, std::string& str)
        {
            uint32_t len = 0;
            in.read(reinterpret_cast<char*>(&len), sizeof(uint32_t));
            if(len > 0)
            {
                str.resize(len);
                in.read(reinterpret_cast<char*>(&str[0]), len);
            }
            else
            {
                str.clear();
            }
        }

        static void WriteString(std::ofstream& out, const std::string& str)
        {
            // ASSERT(str.size() > 0); // Allow empty strings
            uint32_t len = (uint32_t)str.size();
            out.write((char*)&len, sizeof(uint32_t));
            if (len > 0) out.write((char*)str.c_str(), len);
        }
    };

}