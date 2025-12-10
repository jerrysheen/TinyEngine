#pragma once

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