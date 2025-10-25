#define D3D12_API
#define EDITOR



#include "iostream"
#include <unordered_map>
#include <map>
#include <utility>
#include <functional>
#include <string>
#include <variant>
#include <cmath> 
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