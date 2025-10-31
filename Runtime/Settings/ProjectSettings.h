#pragma once
#include <iostream>

namespace EngineCore
{
    class PathSettings
    {
    public:
        static void Initialize();
        static bool sInitialized;
        // 获取各种路径
        static std::string GetProjectRoot(){ return s_ProjectRoot; };
        static std::string GetAssetsPath(){ return s_AssetsPath; };
        static std::string ResolveAssetPath(const std::string& relativePath);
        
        // 便捷方法：解析资源相对路径为绝对路径
        //static std::string ResolveAssetPath(const std::string& relativePath);
        
        // 获取可执行文件路径（用于发布模式）
        static std::string GetExecutablePath();
    private:
        static std::string s_ProjectRoot;
        static std::string s_AssetsPath;        
    };

    class WindowSettings
    {
    public:
        static int s_WindowWidth;
        static int s_WindowHeight;
    };


    class RenderSettings{};

    class ProjectSettings
    {
    public:
        static void Initialize();
        static bool s_Initialized;
    };

}