#include "PreCompiledHeader.h"
#include "ProjectSettings.h"

namespace EngineCore
{
    bool ProjectSettings::s_Initialized = false;
    void ProjectSettings::Initialize()
    {
        PathSettings::Initialize();
        s_Initialized = true;
    }

    std::string PathSettings::s_ProjectRoot = "";
    std::string PathSettings::s_AssetsPath = "";
    void PathSettings::Initialize()
    {
        #ifdef ENGINE_ROOT_PATH
            // 开发模式：使用编译时注入的绝对路径
            s_ProjectRoot = ENGINE_ROOT_PATH;
            s_AssetsPath = ASSETS_PATH;
        #else
            // 发布模式：基于可执行文件位置
            s_ProjectRoot = GetExecutablePath() + "";
            s_AssetsPath = s_ProjectRoot + "/Assets";
        #endif
    }

    std::string PathSettings::GetExecutablePath()
    {
        char buffer[MAX_PATH];
        DWORD length = GetModuleFileNameA(NULL, buffer, MAX_PATH);
        
        // 错误处理：检查是否成功获取路径
        if (length == 0) 
        {
            // 获取失败，返回当前工作目录作为备选
            #ifdef DEBUG_MODE
                printf("[Error] Failed to get executable path, using current directory\n");
            #endif
            return ".";
        }
        
        // 检查路径是否被截断（buffer太小）
        if (length == MAX_PATH) 
        {
            #ifdef DEBUG_MODE
                printf("[Warning] Executable path may be truncated\n");
            #endif
        }
        
        std::string fullPath(buffer, length);
        
        // 查找最后一个斜杠或反斜杠的位置
        size_t lastSlash = fullPath.find_last_of("\\/");
        
        if (lastSlash != std::string::npos) 
        {
            // 返回目录部分（不包含exe文件名）
            return fullPath.substr(0, lastSlash);
        }
        
        // 如果没有找到路径分隔符（不太可能），返回当前目录
        return ".";
    }

    std::string PathSettings::ResolveAssetPath(const std::string& relativePath)
    {
        // 如果已经是绝对路径，直接返回
        if (relativePath.size() >= 2 && relativePath[1] == ':') 
            return relativePath;
        
        // 拼接Assets路径
        std::string fullPath = s_AssetsPath + relativePath;
        
        // 标准化路径
        std::replace(fullPath.begin(), fullPath.end(), '\\', '/');
        
        return fullPath;
    }


    int WindowSettings::s_WindowHeight = 1080;
    int WindowSettings::s_WindowWidth = 1920;

};