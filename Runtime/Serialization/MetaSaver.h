#pragma once
#include <string.h>
#include <fstream> 
#include "json.hpp"
#include "MetaFileSerialization.h"
#include "Serialization/MetaData.h"

namespace EngineCore
{
    class MetaSaver
    {
    public:
        template<typename T>
        static void SaveMetaData(const T* data, const string& relativePath)
        {
            string metaPath = PathSettings::ResolveAssetPath(relativePath);

            // 防御：如果路径中有扩展名，去掉扩展名
            size_t dotPos = metaPath.find_last_of('.');
            if (dotPos != string::npos) {
                metaPath = metaPath.substr(0, dotPos);
            }

            metaPath += ".meta";
            
            json j = *data;

            // 确保目录存在
            std::filesystem::path filePath(metaPath);
            std::filesystem::path dirPath = filePath.parent_path();
            if (!dirPath.empty() && !std::filesystem::exists(dirPath)) {
                std::filesystem::create_directories(dirPath);
            }

            // 写入文件（不存在创建，存在覆盖）
            std::ofstream file(metaPath, std::ios::out | std::ios::trunc);
            if (!file.is_open()) {  
                ASSERT_MSG(false, "Failed to open/create file!");
                return;
            }
            
            file << j.dump(4);
            file.close();
            
            return;
        };
    };
}