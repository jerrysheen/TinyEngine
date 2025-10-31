
// todo： 根据 系统 linux windows android
// 选择默认的API
#define D3D12_API
#define EDITOR

// ========== Windows API（新增） ==========
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN  // 排除一些不常用的 Windows API，加快编译
    #include <Windows.h>
#endif

#include "iostream"
#include <unordered_map>
#include <map>
#include <utility>
#include <functional>
#include <string>
#include <variant>
#include <cmath> 
#include <algorithm>
using std::unordered_map;
using std::string;
using std::vector;
using std::pair;
using std::cout;
using std::endl;
using std::function;

#ifdef _DEBUG
    #define ASSERT_MSG(condition, message) \
        do { \
            if (!(condition)) { \
                std::wcout << L"Assert Failed: " << L#condition << L"\n"; \
                std::wcout << L"Message: " << message << L"\n"; \
                std::wcout << L"File: " << __FILEW__ << L", Line: " << __LINE__ << L"\n"; \
                __debugbreak(); \
            } \
        } while(0)
    
    #define ASSERT(condition) ASSERT_MSG(condition, L"fatal error!")
#else
    #define ASSERT_MSG(condition, message) ((void)0)
    #define ASSERT(condition) ((void)0)
#endif